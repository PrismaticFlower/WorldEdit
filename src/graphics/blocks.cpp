#include "blocks.hpp"
#include "cull_objects.hpp"

#include "gpu/exception.hpp"
#include "gpu/indirect_structures.hpp"

#include "world/blocks/custom_mesh.hpp"
#include "world/blocks/mesh_geometry.hpp"
#include "world/blocks/utility/bounding_box.hpp"
#include "world/utility/entity_group_utilities.hpp"

#include "math/align.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

#include "utility/enum_bitflags.hpp"
#include "utility/string_icompare.hpp"

#include <bit>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::graphics {

namespace {

constexpr uint32 start_custom_mesh_buffer_size = 0x100000; // 1MiB
constexpr uint32 max_custom_mesh_buffer_size = 0x10000000; // 256MiB

struct surface_info {
   uint32 material_index : 8;
   uint32 texture_mode : 4;
   uint32 scaleX : 4;
   uint32 scaleY : 4;
   uint32 rotation : 2;
   uint32 offsetX : 13;
   uint32 offsetY : 13;
   uint32 local_from_world_w_sign : 1;
   uint32 quad_split : 1;
};

static_assert(sizeof(surface_info) == 8);

struct block_instance_description {
   std::array<float3, 4> world_from_local;
   float3x3 adjugate_world_from_local;
   std::array<surface_info, 6> surfaces;
   float3 local_from_world_xyz;
};

static_assert(sizeof(block_instance_description) == 144);

struct block_quad_description {
   std::array<float3, 4> positionWS;
   std::array<float3, 2> normalWS;
   surface_info surface;
};

static_assert(sizeof(block_quad_description) == 80);

enum class block_material_flags : uint32 {
   none = 0b0,
   has_normal_map = 0b10,
   has_detail_map = 0b100,
   has_env_map = 0b1000,
   tile_normal_map = 0b10000,
};

constexpr bool marked_as_enum_bitflag(block_material_flags)
{
   return true;
}

struct block_material {
   block_material_flags flags;
   uint32 diffuse_map_index;
   uint32 normal_map_index;
   uint32 detail_map_index;
   float3 specular_color;
   uint32 env_map_index;
   float2 detail_scale;
   std::array<uint32, 2> padding0;
   float3 env_color;
   uint32 padding;
};

static_assert(sizeof(block_material) == 64);

struct block_indirect_draw {
   uint32 instance_index = 0;
   gpu::indirect_draw_indexed_arguments draw_indexed;
};

static_assert(sizeof(block_indirect_draw) == 24);

struct blocks_ia_buffer {
   std::array<world::block_vertex, 24> cube_vertices = world::block_cube_vertices;
   std::array<std::array<uint16, 3>, 12> cube_indices = world::block_cube_triangles;
   std::array<world::block_vertex, 18> ramp_vertices = world::block_ramp_vertices;
   std::array<std::array<uint16, 3>, 8> ramp_indices = world::block_ramp_triangles;
   std::array<world::block_vertex, 328> hemisphere_vertices =
      world::block_hemisphere_vertices;
   std::array<std::array<uint16, 3>, 510> hemisphere_indices =
      world::block_hemisphere_triangles;
   std::array<world::block_vertex, 16> pyramid_vertices = world::block_pyramid_vertices;
   std::array<std::array<uint16, 3>, 6> pyramid_indices = world::block_pyramid_triangles;
};

auto select_pipeline(const blocks_draw draw, pipeline_library& pipelines) -> gpu::pipeline_handle
{
   switch (draw) {
   case blocks_draw::depth_prepass:
      return pipelines.block_depth_prepass.get();
   case blocks_draw::main:
      return pipelines.block_normal.get();
   case blocks_draw::shadow:
      return pipelines.block_shadow.get();
   }

   std::unreachable();
}

auto select_pipeline_quads(const blocks_draw draw, pipeline_library& pipelines)
   -> gpu::pipeline_handle
{
   switch (draw) {
   case blocks_draw::depth_prepass:
      return pipelines.block_quad_depth_prepass.get();
   case blocks_draw::main:
      return pipelines.block_quad_normal.get();
   case blocks_draw::shadow:
      return pipelines.block_quad_shadow.get();
   }

   std::unreachable();
}

auto select_pipeline_custom_mesh(const blocks_draw draw, pipeline_library& pipelines)
   -> gpu::pipeline_handle
{
   switch (draw) {
   case blocks_draw::depth_prepass:
      return pipelines.block_custom_mesh_depth_prepass.get();
   case blocks_draw::main:
      return pipelines.block_custom_mesh_normal.get();
   case blocks_draw::shadow:
      return pipelines.block_custom_mesh_shadow.get();
   }

   std::unreachable();
}

auto material_flags(const blocks::material& material) noexcept -> block_material_flags
{
   block_material_flags flags = block_material_flags::none;

   if (not material.normal_map.name.empty()) {
      flags |= block_material_flags::has_normal_map;
   }

   if (not material.detail_map.name.empty()) {
      flags |= block_material_flags::has_detail_map;
   }

   if (not material.env_map.name.empty()) {
      flags |= block_material_flags::has_env_map;
   }

   if (material.tile_normal_map) {
      flags |= block_material_flags::tile_normal_map;
   }

   return flags;
}

auto prepare_instances_view(
   blocks_draw draw, const frustum& view_frustum,
   std::span<const float> bbox_min_x, std::span<const float> bbox_min_y,
   std::span<const float> bbox_min_z, std::span<const float> bbox_max_x,
   std::span<const float> bbox_max_y, std::span<const float> bbox_max_z,
   std::span<const bool> hidden, std::span<const int8> layers,
   const world::active_layers active_layers, std::vector<uint32>& culling_storage,
   dynamic_buffer_allocator& dynamic_buffer_allocator) -> blocks::view::instance_list
{
   if (culling_storage.size() < bbox_min_x.size()) {
      culling_storage.resize(bbox_min_x.size());
   }

   uint32 visible_count = 0;

   if (draw == blocks_draw::shadow) {
      cull_objects_shadow_cascade_avx2(view_frustum, bbox_min_x, bbox_min_y,
                                       bbox_min_z, bbox_max_x, bbox_max_y,
                                       bbox_max_z, hidden, layers, active_layers,
                                       visible_count, culling_storage);
   }
   else {
      cull_objects_avx2(view_frustum, bbox_min_x, bbox_min_y, bbox_min_z,
                        bbox_max_x, bbox_max_y, bbox_max_z, hidden, layers,
                        active_layers, visible_count, culling_storage);
   }

   blocks::view::instance_list list;

   if (visible_count > 0) {
      const dynamic_buffer_allocator::allocation& instance_index_allocation =
         dynamic_buffer_allocator.allocate(visible_count * sizeof(uint32));

      std::memcpy(instance_index_allocation.cpu_address, culling_storage.data(),
                  visible_count * sizeof(uint32));

      list.count = visible_count;
      list.instances = instance_index_allocation.gpu_address;
   }

   return list;
}

auto prepare_instances_view(
   blocks_draw draw, const frustum& view_frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z, std::vector<uint32>& culling_storage,
   dynamic_buffer_allocator& dynamic_buffer_allocator) -> blocks::view::instance_list
{
   if (culling_storage.size() < bbox_min_x.size()) {
      culling_storage.resize(bbox_min_x.size());
   }

   uint32 visible_count = 0;

   if (draw == blocks_draw::shadow) {
      cull_objects_shadow_cascade_avx2(view_frustum, bbox_min_x, bbox_min_y,
                                       bbox_min_z, bbox_max_x, bbox_max_y,
                                       bbox_max_z, visible_count, culling_storage);
   }
   else {
      cull_objects_avx2(view_frustum, bbox_min_x, bbox_min_y, bbox_min_z, bbox_max_x,
                        bbox_max_y, bbox_max_z, visible_count, culling_storage);
   }

   blocks::view::instance_list list;

   if (visible_count > 0) {
      const dynamic_buffer_allocator::allocation& instance_index_allocation =
         dynamic_buffer_allocator.allocate(visible_count * sizeof(uint32));

      std::memcpy(instance_index_allocation.cpu_address, culling_storage.data(),
                  visible_count * sizeof(uint32));

      list.count = visible_count;
      list.instances = instance_index_allocation.gpu_address;
   }

   return list;
}

auto prepare_draw_list(
   blocks_draw draw, const frustum& view_frustum,
   std::span<const float> bbox_min_x, std::span<const float> bbox_min_y,
   std::span<const float> bbox_min_z, std::span<const float> bbox_max_x,
   std::span<const float> bbox_max_y, std::span<const float> bbox_max_z,
   std::span<const bool> hidden, std::span<const int8> layers,
   std::span<const world::block_custom_mesh_handle> mesh_handles,
   std::span<const blocks::custom_mesh> meshes,
   const world::active_layers active_layers, std::vector<uint32>& culling_storage,
   dynamic_buffer_allocator& dynamic_buffer_allocator) -> blocks::view::draw_list
{
   if (culling_storage.size() < bbox_min_x.size()) {
      culling_storage.resize(bbox_min_x.size());
   }

   uint32 visible_count = 0;

   if (draw == blocks_draw::shadow) {
      cull_objects_shadow_cascade_avx2(view_frustum, bbox_min_x, bbox_min_y,
                                       bbox_min_z, bbox_max_x, bbox_max_y,
                                       bbox_max_z, hidden, layers, active_layers,
                                       visible_count, culling_storage);
   }
   else {
      cull_objects_avx2(view_frustum, bbox_min_x, bbox_min_y, bbox_min_z,
                        bbox_max_x, bbox_max_y, bbox_max_z, hidden, layers,
                        active_layers, visible_count, culling_storage);
   }

   blocks::view::draw_list list;

   if (visible_count > 0) {
      const dynamic_buffer_allocator::allocation& draw_allocation =
         dynamic_buffer_allocator.allocate(visible_count * sizeof(block_indirect_draw));

      std::byte* upload_ptr = draw_allocation.cpu_address;

      for (uint32 i = 0; i < visible_count; ++i) {
         const uint32 instance_index = culling_storage[i];

         const blocks::custom_mesh& mesh =
            meshes[world::blocks_custom_mesh_library::unpack_pool_index(mesh_handles[instance_index])];

         const block_indirect_draw indirect_draw = {
            .instance_index = instance_index,
            .draw_indexed = {mesh.index_count, 1, mesh.start_index_location,
                             mesh.base_vertex_location, 0},
         };

         std::memcpy(upload_ptr, &indirect_draw, sizeof(block_indirect_draw));

         upload_ptr += sizeof(block_indirect_draw);
      }

      list.count = visible_count;
      list.draw_buffer = draw_allocation.resource;
      list.draw_buffer_offset = draw_allocation.offset;
   }

   return list;
}

auto prepare_draw_list(
   blocks_draw draw, const frustum& view_frustum,
   std::span<const float> bbox_min_x, std::span<const float> bbox_min_y,
   std::span<const float> bbox_min_z, std::span<const float> bbox_max_x,
   std::span<const float> bbox_max_y, std::span<const float> bbox_max_z,
   std::span<const world::block_custom_mesh_handle> mesh_handles,
   std::span<const blocks::custom_mesh> meshes, std::vector<uint32>& culling_storage,
   dynamic_buffer_allocator& dynamic_buffer_allocator) -> blocks::view::draw_list
{
   if (culling_storage.size() < bbox_min_x.size()) {
      culling_storage.resize(bbox_min_x.size());
   }

   uint32 visible_count = 0;

   if (draw == blocks_draw::shadow) {
      cull_objects_shadow_cascade_avx2(view_frustum, bbox_min_x, bbox_min_y,
                                       bbox_min_z, bbox_max_x, bbox_max_y,
                                       bbox_max_z, visible_count, culling_storage);
   }
   else {
      cull_objects_avx2(view_frustum, bbox_min_x, bbox_min_y, bbox_min_z, bbox_max_x,
                        bbox_max_y, bbox_max_z, visible_count, culling_storage);
   }

   blocks::view::draw_list list;

   if (visible_count > 0) {
      const dynamic_buffer_allocator::allocation& draw_allocation =
         dynamic_buffer_allocator.allocate(visible_count * sizeof(block_indirect_draw));

      std::byte* upload_ptr = draw_allocation.cpu_address;

      for (uint32 i = 0; i < visible_count; ++i) {
         const uint32 instance_index = culling_storage[i];

         const blocks::custom_mesh& mesh =
            meshes[world::blocks_custom_mesh_library::unpack_pool_index(mesh_handles[instance_index])];

         const block_indirect_draw indirect_draw = {
            .instance_index = instance_index,
            .draw_indexed = {mesh.index_count, 1, mesh.start_index_location,
                             mesh.base_vertex_location, 0},
         };

         std::memcpy(upload_ptr, &indirect_draw, sizeof(block_indirect_draw));

         upload_ptr += sizeof(block_indirect_draw);
      }

      list.count = visible_count;
      list.draw_buffer = draw_allocation.resource;
      list.draw_buffer_offset = draw_allocation.offset;
   }

   return list;
}
}

blocks::blocks(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager, root_signature_library& root_signatures)
   : _device{device},
     _custom_mesh_allocator{max_custom_mesh_buffer_size,
                            sizeof(world::block_vertex), world::max_blocks}
{
   _blocks_ia_buffer = {_device.create_buffer({.size = sizeof(blocks_ia_buffer),
                                               .debug_name =
                                                  "Blocks IA Buffer"},
                                              gpu::heap_type::default_),
                        _device};

   _blocks_materials_buffer =
      {_device.create_buffer({.size = sizeof(block_material) * world::max_block_materials,
                              .debug_name = "Blocks Materials"},
                             gpu::heap_type::default_),
       _device};

   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   {
      auto upload_data =
         dynamic_buffer_allocator.allocate_and_copy(blocks_ia_buffer{});

      command_list->copy_buffer_region(_blocks_ia_buffer.get(), 0,
                                       upload_data.resource, upload_data.offset,
                                       sizeof(blocks_ia_buffer));
   }

   {
      auto upload_data = dynamic_buffer_allocator.allocate(
         sizeof(block_material) * world::max_block_materials);

      const block_material default_material{
         .flags = block_material_flags::none,
         .diffuse_map_index = texture_manager.null_diffuse_map()->srv_srgb.index,
         .normal_map_index = texture_manager.null_normal_map()->srv.index,
         .detail_map_index = texture_manager.null_detail_map()->srv.index,
         .specular_color = {0.0f, 0.0f, 0.0f},
         .env_map_index = texture_manager.null_cube_map()->srv_srgb.index,
         .detail_scale = {1.0f, 1.0f},
         .env_color = {0.0f, 0.0f, 0.0f},
      };

      for (uint32 i = 0; i < world::max_block_materials; ++i) {
         std::memcpy(upload_data.cpu_address + i * sizeof(block_material),
                     &default_material, sizeof(block_material));
      }

      command_list->copy_buffer_region(_blocks_materials_buffer.get(), 0,
                                       upload_data.resource, upload_data.offset,
                                       sizeof(block_material) * world::max_block_materials);
   }

   command_list->close();

   device.background_copy_queue.execute_command_lists(command_list.get());

   _materials.resize(world::max_block_materials);

   for (material& material : _materials) {
      material.diffuse_map.texture = texture_manager.null_diffuse_map();
      material.normal_map.texture = texture_manager.null_normal_map();
      material.detail_map.texture = texture_manager.null_detail_map();
      material.env_map.texture = texture_manager.null_cube_map();
   }

   const std::array<gpu::indirect_argument_desc, 2> command_signature_arguments = {
      gpu::indirect_argument_desc{.type = gpu::indirect_argument_type::constant,
                                  .constant =
                                     {
                                        .root_parameter_index =
                                           rs::block_custom_mesh::instance_index,
                                        .dest_offset_in_32bit_values = 0,
                                        .num_32bit_values_to_set = 1,
                                     }},
      gpu::indirect_argument_desc{.type = gpu::indirect_argument_type::draw_indexed},
   };

   _custom_mesh_command_signature =
      {_device.create_command_signature({.byte_stride = sizeof(block_indirect_draw),
                                         .argument_descs = command_signature_arguments},
                                        root_signatures.block_custom_mesh.get()),
       _device};
}

void blocks::update(const world::blocks& blocks, const world::entity_group* entity_group,
                    gpu::copy_command_list& command_list,
                    dynamic_buffer_allocator& dynamic_buffer_allocator,
                    texture_manager& texture_manager)
{
   bool custom_mesh_buffer_recreated = false;

   if (_boxes_instance_data_capacity < blocks.boxes.size()) {
      const gpu::unique_resource_handle old_boxes_instance_data =
         std::move(_boxes_instance_data);
      const uint64 old_boxes_instance_data_capacity = _boxes_instance_data_capacity;

      _boxes_instance_data_capacity = blocks.boxes.size() * 16180 / 10000;
      _boxes_instance_data =
         {_device.create_buffer({.size = _boxes_instance_data_capacity *
                                         sizeof(block_instance_description),
                                 .debug_name = "World blocks (Boxes)"},
                                gpu::heap_type::default_),
          _device};

      if (old_boxes_instance_data_capacity != 0) {
         command_list.copy_buffer_region(_boxes_instance_data.get(), 0,
                                         old_boxes_instance_data.get(), 0,
                                         old_boxes_instance_data_capacity *
                                            sizeof(block_instance_description));
         command_list.reference_resource(old_boxes_instance_data.get());
      }
   }

   if (_ramps_instance_data_capacity < blocks.ramps.size()) {
      const gpu::unique_resource_handle old_ramps_instance_data =
         std::move(_ramps_instance_data);
      const uint64 old_ramps_instance_data_capacity = _ramps_instance_data_capacity;

      _ramps_instance_data_capacity = blocks.ramps.size() * 16180 / 10000;
      _ramps_instance_data =
         {_device.create_buffer({.size = _ramps_instance_data_capacity *
                                         sizeof(block_instance_description),
                                 .debug_name = "World blocks (Ramps)"},
                                gpu::heap_type::default_),
          _device};

      if (old_ramps_instance_data_capacity != 0) {
         command_list.copy_buffer_region(_ramps_instance_data.get(), 0,
                                         old_ramps_instance_data.get(), 0,
                                         old_ramps_instance_data_capacity *
                                            sizeof(block_instance_description));
         command_list.reference_resource(old_ramps_instance_data.get());
      }
   }

   if (_quads_instance_data_capacity < blocks.quads.size()) {
      const gpu::unique_resource_handle old_quads_instance_data =
         std::move(_quads_instance_data);
      const uint64 old_quads_instance_data_capacity = _quads_instance_data_capacity;

      _quads_instance_data_capacity = blocks.quads.size() * 16180 / 10000;
      _quads_instance_data =
         {_device.create_buffer({.size = _quads_instance_data_capacity *
                                         sizeof(block_quad_description),
                                 .debug_name = "World blocks (Quads)"},
                                gpu::heap_type::default_),
          _device};

      if (old_quads_instance_data_capacity != 0) {
         command_list.copy_buffer_region(_quads_instance_data.get(), 0,
                                         old_quads_instance_data.get(), 0,
                                         old_quads_instance_data_capacity *
                                            sizeof(block_quad_description));
         command_list.reference_resource(old_quads_instance_data.get());
      }
   }

   if (_custom_instance_data_capacity < blocks.custom.size()) {
      const gpu::unique_resource_handle old_custom_instance_data =
         std::move(_custom_instance_data);
      const uint64 old_custom_instance_data_capacity = _custom_instance_data_capacity;

      _custom_instance_data_capacity = blocks.custom.size() * 16180 / 10000;
      _custom_instance_data =
         {_device.create_buffer({.size = _custom_instance_data_capacity *
                                         sizeof(block_instance_description),
                                 .debug_name = "World blocks (Custom Blocks)"},
                                gpu::heap_type::default_),
          _device};

      if (old_custom_instance_data_capacity != 0) {
         command_list.copy_buffer_region(_custom_instance_data.get(), 0,
                                         old_custom_instance_data.get(), 0,
                                         old_custom_instance_data_capacity *
                                            sizeof(block_instance_description));
         command_list.reference_resource(old_custom_instance_data.get());
      }
   }

   if (_hemispheres_instance_data_capacity < blocks.hemispheres.size()) {
      const gpu::unique_resource_handle old_hemispheres_instance_data =
         std::move(_hemispheres_instance_data);
      const uint64 old_hemispheres_instance_data_capacity =
         _hemispheres_instance_data_capacity;

      _hemispheres_instance_data_capacity = blocks.hemispheres.size() * 16180 / 10000;
      _hemispheres_instance_data =
         {_device.create_buffer({.size = _hemispheres_instance_data_capacity *
                                         sizeof(block_instance_description),
                                 .debug_name = "World blocks (Hemispheres)"},
                                gpu::heap_type::default_),
          _device};

      if (old_hemispheres_instance_data_capacity != 0) {
         command_list.copy_buffer_region(_hemispheres_instance_data.get(), 0,
                                         old_hemispheres_instance_data.get(), 0,
                                         old_hemispheres_instance_data_capacity *
                                            sizeof(block_instance_description));
         command_list.reference_resource(old_hemispheres_instance_data.get());
      }
   }

   if (_pyramids_instance_data_capacity < blocks.pyramids.size()) {
      const gpu::unique_resource_handle old_pyramids_instance_data =
         std::move(_pyramids_instance_data);
      const uint64 old_pyramids_instance_data_capacity =
         _pyramids_instance_data_capacity;

      _pyramids_instance_data_capacity = blocks.pyramids.size() * 16180 / 10000;
      _pyramids_instance_data =
         {_device.create_buffer({.size = _pyramids_instance_data_capacity *
                                         sizeof(block_instance_description),
                                 .debug_name = "World blocks (Pyramids)"},
                                gpu::heap_type::default_),
          _device};

      if (old_pyramids_instance_data_capacity != 0) {
         command_list.copy_buffer_region(_pyramids_instance_data.get(), 0,
                                         old_pyramids_instance_data.get(), 0,
                                         old_pyramids_instance_data_capacity *
                                            sizeof(block_instance_description));
         command_list.reference_resource(old_pyramids_instance_data.get());
      }
   }

   for (const world::blocks_custom_mesh_library::event event :
        blocks.custom_meshes.events()) {
      using event_type = world::blocks_custom_mesh_library::event_type;

      switch (event.type) {
      case event_type::cleared: {
         for (custom_mesh& mesh : _custom_meshes) {
            if (mesh.ia_allocation) {
               _custom_mesh_allocator.free(mesh.ia_allocation);

               mesh.ia_allocation = {};
            }
         }

         _custom_meshes.clear();
      } break;
      case event_type::mesh_added: {
         const uint32 mesh_index =
            blocks.custom_meshes.unpack_pool_index(event.handle);

         if (mesh_index >= _custom_meshes.size()) {
            _custom_meshes.reserve(blocks.custom_meshes.pool_size());
            _custom_meshes.resize(mesh_index + 1);
         }

         const world::block_custom_mesh& block = blocks.custom_meshes[event.handle];
         custom_mesh& mesh = _custom_meshes[mesh_index];

         mesh.vertex_count = static_cast<uint32>(block.vertices.size());
         mesh.index_count = static_cast<uint32>(block.triangles.size() * 3);

         const uint32 mesh_size = mesh.vertex_count * sizeof(world::block_vertex) +
                                  mesh.index_count * sizeof(uint16);

         mesh.ia_allocation = _custom_mesh_allocator.allocate(mesh_size);

         if (not mesh.ia_allocation) {
            throw gpu::exception{
               gpu::error::out_of_memory,
               "Out of memory for blocks with custom meshes!"};
         }

         if ((mesh.ia_allocation.offset + mesh_size) > _custom_mesh_buffer_capacity) {
            const gpu::unique_resource_handle old_custom_mesh_buffer =
               std::move(_custom_mesh_buffer);
            const uint64 old_custom_mesh_buffer_capacity = _custom_mesh_buffer_capacity;

            _custom_mesh_buffer_capacity = _custom_mesh_buffer_capacity == 0
                                              ? start_custom_mesh_buffer_size
                                              : _custom_mesh_buffer_capacity * 2;
            _custom_mesh_buffer =
               {_device.create_buffer({.size = _custom_mesh_buffer_capacity,
                                       .debug_name =
                                          "World blocks (Custom Meshes)"},
                                      gpu::heap_type::default_),
                _device};

            if (old_custom_mesh_buffer_capacity != 0) {
               if (custom_mesh_buffer_recreated) {
                  [[likely]] if (_device.supports_enhanced_barriers()) {
                     command_list.deferred_barrier(
                        gpu::buffer_barrier{.sync_before = gpu::barrier_sync::copy,
                                            .sync_after = gpu::barrier_sync::copy,
                                            .access_before = gpu::barrier_access::copy_dest,
                                            .access_after = gpu::barrier_access::copy_source,
                                            .resource = old_custom_mesh_buffer.get()});
                  }
                  else {
                     command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
                        .resource = old_custom_mesh_buffer.get(),
                        .state_before = gpu::legacy_resource_state::copy_dest,
                        .state_after = gpu::legacy_resource_state::copy_source});
                  }

                  command_list.flush_barriers();
               }

               command_list.copy_buffer_region(_custom_mesh_buffer.get(), 0,
                                               old_custom_mesh_buffer.get(), 0,
                                               old_custom_mesh_buffer_capacity);
               command_list.reference_resource(old_custom_mesh_buffer.get());

               custom_mesh_buffer_recreated = true;
            }
         }

         mesh.base_vertex_location =
            mesh.ia_allocation.offset / sizeof(world::block_vertex);
         mesh.start_index_location =
            (mesh.ia_allocation.offset +
             mesh.vertex_count * sizeof(world::block_vertex)) /
            sizeof(uint16);

         const uint32 vertices_size = mesh.vertex_count * sizeof(world::block_vertex);
         const uint64 inidces_size = mesh.index_count * sizeof(uint16);

         auto vertices_upload = dynamic_buffer_allocator.allocate(vertices_size);

         std::memcpy(vertices_upload.cpu_address, block.vertices.data(), vertices_size);

         auto indices_upload = dynamic_buffer_allocator.allocate(inidces_size);

         std::memcpy(indices_upload.cpu_address, block.triangles.data(), inidces_size);

         command_list.copy_buffer_region(_custom_mesh_buffer.get(),
                                         mesh.ia_allocation.offset,
                                         vertices_upload.resource,
                                         vertices_upload.offset, vertices_size);

         command_list.copy_buffer_region(_custom_mesh_buffer.get(),
                                         mesh.ia_allocation.offset + vertices_size,
                                         indices_upload.resource,
                                         indices_upload.offset, inidces_size);
      } break;
      case event_type::mesh_removed: {
         const uint32 mesh_index =
            blocks.custom_meshes.unpack_pool_index(event.handle);

         if (mesh_index >= _custom_meshes.size()) continue;

         custom_mesh& mesh = _custom_meshes[mesh_index];

         if (mesh.ia_allocation) {
            _custom_mesh_allocator.free(mesh.ia_allocation);

            mesh.ia_allocation = {};
         }
      } break;
      }
   }

   for (const world::blocks_dirty_range& range : blocks.boxes.dirty) {
      const uint32 range_size = range.end - range.begin;

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(range_size *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = range.begin; block_index < range.end; ++block_index) {
         const world::block_description_box& block =
            blocks.boxes.description[block_index];

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block.rotation);
         const float4x4 world_from_local = rotation * scale;

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block.position;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);
      }

      command_list.copy_buffer_region(_boxes_instance_data.get(),
                                      range.begin * sizeof(block_instance_description),
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      range_size * sizeof(block_instance_description));
   }

   for (const world::blocks_dirty_range& range : blocks.ramps.dirty) {
      const uint32 range_size = range.end - range.begin;

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(range_size *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = range.begin; block_index < range.end; ++block_index) {
         const world::block_description_ramp& block =
            blocks.ramps.description[block_index];

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block.rotation);
         const float4x4 world_from_local = rotation * scale;

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block.position;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);
      }

      command_list.copy_buffer_region(_ramps_instance_data.get(),
                                      range.begin * sizeof(block_instance_description),
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      range_size * sizeof(block_instance_description));
   }

   for (const world::blocks_dirty_range& range : blocks.quads.dirty) {
      const uint32 range_size = range.end - range.begin;

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(range_size * sizeof(block_quad_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = range.begin; block_index < range.end; ++block_index) {
         const world::block_description_quad& block =
            blocks.quads.description[block_index];

         block_quad_description description;

         description.positionWS = block.vertices;
         description.surface = {
            .material_index = block.surface_materials[0],
            .texture_mode = static_cast<uint32>(block.surface_texture_mode[0]),
            .scaleX = static_cast<uint32>(block.surface_texture_scale[0][0] + 7),
            .scaleY = static_cast<uint32>(block.surface_texture_scale[0][1] + 7),
            .rotation = static_cast<uint32>(block.surface_texture_rotation[0]),
            .offsetX = block.surface_texture_offset[0][0],
            .offsetY = block.surface_texture_offset[0][1],
            .quad_split = static_cast<uint32>(block.quad_split),
         };

         const std::array<std::array<uint16, 3>, 2>& quad_triangles =
            block.quad_split == world::block_quad_split::regular
               ? world::block_quad_triangles
               : world::block_quad_alternate_triangles;

         description.normalWS[0] =
            normalize(cross(description.positionWS[quad_triangles[0][1]] -
                               description.positionWS[quad_triangles[0][0]],
                            description.positionWS[quad_triangles[0][2]] -
                               description.positionWS[quad_triangles[0][0]]));
         description.normalWS[1] =
            normalize(cross(description.positionWS[quad_triangles[1][1]] -
                               description.positionWS[quad_triangles[1][0]],
                            description.positionWS[quad_triangles[1][2]] -
                               description.positionWS[quad_triangles[1][0]]));

         std::memcpy(upload_ptr, &description, sizeof(block_quad_description));

         upload_ptr += sizeof(block_quad_description);
      }

      command_list.copy_buffer_region(_quads_instance_data.get(),
                                      range.begin * sizeof(block_quad_description),
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      range_size * sizeof(block_quad_description));
   }

   for (const world::blocks_dirty_range& range : blocks.custom.dirty) {
      const uint32 range_size = range.end - range.begin;

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(range_size *
                                           sizeof(block_instance_description));
      std::byte* instance_upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = range.begin; block_index < range.end; ++block_index) {
         const world::block_description_custom& block =
            blocks.custom.description[block_index];

         const float4x4 world_from_local = to_matrix(block.rotation);

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block.position;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(instance_upload_ptr, &description,
                     sizeof(block_instance_description));

         instance_upload_ptr += sizeof(block_instance_description);
      }

      command_list.copy_buffer_region(_custom_instance_data.get(),
                                      range.begin * sizeof(block_instance_description),
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      range_size * sizeof(block_instance_description));
   }

   for (const world::blocks_dirty_range& range : blocks.hemispheres.dirty) {
      const uint32 range_size = range.end - range.begin;

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(range_size *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = range.begin; block_index < range.end; ++block_index) {
         const world::block_description_hemisphere& block =
            blocks.hemispheres.description[block_index];

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block.rotation);
         const float4x4 world_from_local = rotation * scale;

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block.position;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);
      }

      command_list.copy_buffer_region(_hemispheres_instance_data.get(),
                                      range.begin * sizeof(block_instance_description),
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      range_size * sizeof(block_instance_description));
   }

   for (const world::blocks_dirty_range& range : blocks.pyramids.dirty) {
      const uint32 range_size = range.end - range.begin;

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(range_size *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = range.begin; block_index < range.end; ++block_index) {
         const world::block_description_pyramid& block =
            blocks.pyramids.description[block_index];

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block.rotation);
         const float4x4 world_from_local = rotation * scale;

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block.position;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);
      }

      command_list.copy_buffer_region(_pyramids_instance_data.get(),
                                      range.begin * sizeof(block_instance_description),
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      range_size * sizeof(block_instance_description));
   }

   for (const world::blocks_dirty_range& range : blocks.materials_dirty) {
      const uint32 range_size = range.end - range.begin;

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(range_size * sizeof(block_material));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 material_index = range.begin; material_index < range.end;
           ++material_index) {
         const world::block_material& world_material =
            blocks.materials[material_index];
         material& material = _materials[material_index];

         material.diffuse_map.update(world_material.diffuse_map, texture_manager,
                                     texture_manager.null_diffuse_map());
         material.normal_map.update(world_material.normal_map, texture_manager,
                                    texture_manager.null_normal_map());
         material.detail_map.update(world_material.detail_map, texture_manager,
                                    texture_manager.null_detail_map());
         material.env_map.update(world_material.env_map, texture_manager,
                                 texture_manager.null_cube_map());

         material.detail_tiling = world_material.detail_tiling;
         material.tile_normal_map = world_material.tile_normal_map;
         material.specular_lighting = world_material.specular_lighting;

         material.specular_color = world_material.specular_color;

         const block_material constants{
            .flags = material_flags(material),
            .diffuse_map_index = material.diffuse_map.texture->srv_srgb.index,
            .normal_map_index = material.normal_map.texture->srv.index,
            .detail_map_index = material.detail_map.texture->srv.index,
            .specular_color = material.specular_lighting ? material.specular_color
                                                         : float3{0.0f, 0.0f, 0.0f},
            .env_map_index = material.env_map.texture->srv_srgb.index,
            .detail_scale = {std::max(material.detail_tiling[0], uint8{1}) * 1.0f,
                             std::max(material.detail_tiling[1], uint8{1}) * 1.0f},
            .env_color = not material.env_map.name.empty() ? material.specular_color
                                                           : float3{1.0f, 1.0f, 1.0f},
         };

         std::memcpy(upload_ptr, &constants, sizeof(block_material));

         upload_ptr += sizeof(block_material);
      }

      command_list.copy_buffer_region(_blocks_materials_buffer.get(),
                                      range.begin * sizeof(block_material),
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      range_size * sizeof(block_material));
   }

   [[unlikely]] if (entity_group and
                    not world::is_entity_group_blocks_empty(*entity_group)) {
      if (not _dynamic_blocks) {
         _dynamic_blocks = std::make_unique<dynamic_blocks>();
      }

      _dynamic_blocks->update(*entity_group, _device, command_list,
                              dynamic_buffer_allocator, texture_manager);
   }
   else if (_dynamic_blocks) {
      _dynamic_blocks = nullptr;
   }
}

auto blocks::prepare_view(blocks_draw draw, const world::blocks& blocks,
                          const world::entity_group* entity_group,
                          const frustum& view_frustum,
                          const world::active_layers active_layers,
                          dynamic_buffer_allocator& dynamic_buffer_allocator) -> view
{
   view view;

   view.boxes =
      prepare_instances_view(draw, view_frustum, blocks.boxes.bbox.min_x,
                             blocks.boxes.bbox.min_y, blocks.boxes.bbox.min_z,
                             blocks.boxes.bbox.max_x, blocks.boxes.bbox.max_y,
                             blocks.boxes.bbox.max_z, blocks.boxes.hidden,
                             blocks.boxes.layer, active_layers,
                             _TEMP_culling_storage, dynamic_buffer_allocator);
   view.ramps =
      prepare_instances_view(draw, view_frustum, blocks.ramps.bbox.min_x,
                             blocks.ramps.bbox.min_y, blocks.ramps.bbox.min_z,
                             blocks.ramps.bbox.max_x, blocks.ramps.bbox.max_y,
                             blocks.ramps.bbox.max_z, blocks.ramps.hidden,
                             blocks.ramps.layer, active_layers,
                             _TEMP_culling_storage, dynamic_buffer_allocator);
   view.quads =
      prepare_instances_view(draw, view_frustum, blocks.quads.bbox.min_x,
                             blocks.quads.bbox.min_y, blocks.quads.bbox.min_z,
                             blocks.quads.bbox.max_x, blocks.quads.bbox.max_y,
                             blocks.quads.bbox.max_z, blocks.quads.hidden,
                             blocks.quads.layer, active_layers,
                             _TEMP_culling_storage, dynamic_buffer_allocator);

   view.custom =
      prepare_draw_list(draw, view_frustum, blocks.custom.bbox.min_x,
                        blocks.custom.bbox.min_y, blocks.custom.bbox.min_z,
                        blocks.custom.bbox.max_x, blocks.custom.bbox.max_y,
                        blocks.custom.bbox.max_z, blocks.custom.hidden,
                        blocks.custom.layer, blocks.custom.mesh, _custom_meshes,
                        active_layers, _TEMP_culling_storage,
                        dynamic_buffer_allocator);
   view.hemispheres =
      prepare_instances_view(draw, view_frustum, blocks.hemispheres.bbox.min_x,
                             blocks.hemispheres.bbox.min_y,
                             blocks.hemispheres.bbox.min_z,
                             blocks.hemispheres.bbox.max_x,
                             blocks.hemispheres.bbox.max_y,
                             blocks.hemispheres.bbox.max_z, blocks.hemispheres.hidden,
                             blocks.hemispheres.layer, active_layers,
                             _TEMP_culling_storage, dynamic_buffer_allocator);
   view.pyramids =
      prepare_instances_view(draw, view_frustum, blocks.pyramids.bbox.min_x,
                             blocks.pyramids.bbox.min_y, blocks.pyramids.bbox.min_z,
                             blocks.pyramids.bbox.max_x, blocks.pyramids.bbox.max_y,
                             blocks.pyramids.bbox.max_z, blocks.pyramids.hidden,
                             blocks.pyramids.layer, active_layers,
                             _TEMP_culling_storage, dynamic_buffer_allocator);

   [[unlikely]] if (entity_group and _dynamic_blocks) {
      view.dynamic_boxes =
         prepare_instances_view(draw, view_frustum, _dynamic_blocks->boxes_bbox.min_x,
                                _dynamic_blocks->boxes_bbox.min_y,
                                _dynamic_blocks->boxes_bbox.min_z,
                                _dynamic_blocks->boxes_bbox.max_x,
                                _dynamic_blocks->boxes_bbox.max_y,
                                _dynamic_blocks->boxes_bbox.max_z,
                                _TEMP_culling_storage, dynamic_buffer_allocator);

      view.dynamic_ramps =
         prepare_instances_view(draw, view_frustum, _dynamic_blocks->ramps_bbox.min_x,
                                _dynamic_blocks->ramps_bbox.min_y,
                                _dynamic_blocks->ramps_bbox.min_z,
                                _dynamic_blocks->ramps_bbox.max_x,
                                _dynamic_blocks->ramps_bbox.max_y,
                                _dynamic_blocks->ramps_bbox.max_z,
                                _TEMP_culling_storage, dynamic_buffer_allocator);

      view.dynamic_quads =
         prepare_instances_view(draw, view_frustum, _dynamic_blocks->quads_bbox.min_x,
                                _dynamic_blocks->quads_bbox.min_y,
                                _dynamic_blocks->quads_bbox.min_z,
                                _dynamic_blocks->quads_bbox.max_x,
                                _dynamic_blocks->quads_bbox.max_y,
                                _dynamic_blocks->quads_bbox.max_z,
                                _TEMP_culling_storage, dynamic_buffer_allocator);

      view.dynamic_custom =
         prepare_draw_list(draw, view_frustum, _dynamic_blocks->custom_bbox.min_x,
                           _dynamic_blocks->custom_bbox.min_y,
                           _dynamic_blocks->custom_bbox.min_z,
                           _dynamic_blocks->custom_bbox.max_x,
                           _dynamic_blocks->custom_bbox.max_y,
                           _dynamic_blocks->custom_bbox.max_z,
                           entity_group->blocks.custom.mesh, _custom_meshes,
                           _TEMP_culling_storage, dynamic_buffer_allocator);

      view.dynamic_hemispheres =
         prepare_instances_view(draw, view_frustum,
                                _dynamic_blocks->hemispheres_bbox.min_x,
                                _dynamic_blocks->hemispheres_bbox.min_y,
                                _dynamic_blocks->hemispheres_bbox.min_z,
                                _dynamic_blocks->hemispheres_bbox.max_x,
                                _dynamic_blocks->hemispheres_bbox.max_y,
                                _dynamic_blocks->hemispheres_bbox.max_z,
                                _TEMP_culling_storage, dynamic_buffer_allocator);

      view.dynamic_pyramids =
         prepare_instances_view(draw, view_frustum,
                                _dynamic_blocks->pyramids_bbox.min_x,
                                _dynamic_blocks->pyramids_bbox.min_y,
                                _dynamic_blocks->pyramids_bbox.min_z,
                                _dynamic_blocks->pyramids_bbox.max_x,
                                _dynamic_blocks->pyramids_bbox.max_y,
                                _dynamic_blocks->pyramids_bbox.max_z,
                                _TEMP_culling_storage, dynamic_buffer_allocator);
   }

   return view;
}

void blocks::draw(blocks_draw draw, const view& view,
                  gpu_virtual_address frame_constant_buffer_view,
                  gpu_virtual_address lights_constant_buffer_view,
                  gpu::graphics_command_list& command_list,
                  root_signature_library& root_signatures,
                  pipeline_library& pipelines) const
{
   command_list.set_graphics_root_signature(root_signatures.block.get());

   command_list.set_graphics_cbv(rs::block::frame_cbv, frame_constant_buffer_view);
   command_list.set_graphics_cbv(rs::block::lights_cbv, lights_constant_buffer_view);
   command_list.set_graphics_cbv(rs::block::materials_cbv,
                                 _device.get_gpu_virtual_address(
                                    _blocks_materials_buffer.get()));

   command_list.set_pipeline_state(select_pipeline(draw, pipelines));

   const gpu_virtual_address ia_address =
      _device.get_gpu_virtual_address(_blocks_ia_buffer.get());

   if (view.boxes.count > 0) {
      command_list.set_graphics_srv(rs::block::instances_index_srv, view.boxes.instances);
      command_list.set_graphics_srv(rs::block::instances_srv,
                                    _device.get_gpu_virtual_address(
                                       _boxes_instance_data.get()));

      command_list.ia_set_index_buffer({
         .buffer_location = ia_address + offsetof(blocks_ia_buffer, cube_indices),
         .size_in_bytes = sizeof(blocks_ia_buffer::cube_indices),
      });
      command_list.ia_set_vertex_buffers(
         0, gpu::vertex_buffer_view{
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, cube_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::cube_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            });

      command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::cube_indices) / 2,
                                          view.boxes.count, 0, 0, 0);
   }

   if (view.ramps.count > 0) {
      command_list.set_graphics_srv(rs::block::instances_index_srv, view.ramps.instances);
      command_list.set_graphics_srv(rs::block::instances_srv,
                                    _device.get_gpu_virtual_address(
                                       _ramps_instance_data.get()));

      command_list.ia_set_index_buffer({
         .buffer_location = ia_address + offsetof(blocks_ia_buffer, ramp_indices),
         .size_in_bytes = sizeof(blocks_ia_buffer::ramp_indices),
      });
      command_list.ia_set_vertex_buffers(
         0, gpu::vertex_buffer_view{
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, ramp_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::ramp_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            });

      command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::ramp_indices) / 2,
                                          view.ramps.count, 0, 0, 0);
   }

   if (view.hemispheres.count > 0) {
      command_list.set_graphics_srv(rs::block::instances_index_srv,
                                    view.hemispheres.instances);
      command_list.set_graphics_srv(rs::block::instances_srv,
                                    _device.get_gpu_virtual_address(
                                       _hemispheres_instance_data.get()));

      command_list.ia_set_index_buffer({
         .buffer_location = ia_address + offsetof(blocks_ia_buffer, hemisphere_indices),
         .size_in_bytes = sizeof(blocks_ia_buffer::hemisphere_indices),
      });
      command_list.ia_set_vertex_buffers(
         0, gpu::vertex_buffer_view{
               .buffer_location =
                  ia_address + offsetof(blocks_ia_buffer, hemisphere_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::hemisphere_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            });

      command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::hemisphere_indices) / 2,
                                          view.hemispheres.count, 0, 0, 0);
   }

   if (view.pyramids.count > 0) {
      command_list.set_graphics_srv(rs::block::instances_index_srv,
                                    view.pyramids.instances);
      command_list.set_graphics_srv(rs::block::instances_srv,
                                    _device.get_gpu_virtual_address(
                                       _pyramids_instance_data.get()));

      command_list.ia_set_index_buffer({
         .buffer_location = ia_address + offsetof(blocks_ia_buffer, pyramid_indices),
         .size_in_bytes = sizeof(blocks_ia_buffer::pyramid_indices),
      });
      command_list.ia_set_vertex_buffers(
         0, gpu::vertex_buffer_view{
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, pyramid_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::pyramid_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            });

      command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::pyramid_indices) / 2,
                                          view.pyramids.count, 0, 0, 0);
   }

   if (view.quads.count > 0) {
      command_list.set_pipeline_state(select_pipeline_quads(draw, pipelines));

      command_list.set_graphics_srv(rs::block::instances_index_srv, view.quads.instances);
      command_list.set_graphics_srv(rs::block::instances_srv,
                                    _device.get_gpu_virtual_address(
                                       _quads_instance_data.get()));

      command_list.draw_instanced(view.quads.count * 6, 1, 0, 0);
   }

   [[unlikely]] if (_dynamic_blocks) {
      if (view.quads.count > 0) {
         command_list.set_pipeline_state(select_pipeline(draw, pipelines));
      }

      command_list.set_graphics_cbv(rs::block::materials_cbv,
                                    _device.get_gpu_virtual_address(
                                       _dynamic_blocks->materials_buffer.get()));

      if (view.dynamic_boxes.count > 0) {
         command_list.set_graphics_srv(rs::block::instances_index_srv,
                                       view.dynamic_boxes.instances);
         command_list.set_graphics_srv(rs::block::instances_srv,
                                       _device.get_gpu_virtual_address(
                                          _dynamic_blocks->boxes_instance_data.get()));

         command_list.ia_set_index_buffer({
            .buffer_location = ia_address + offsetof(blocks_ia_buffer, cube_indices),
            .size_in_bytes = sizeof(blocks_ia_buffer::cube_indices),
         });
         command_list.ia_set_vertex_buffers(
            0, gpu::vertex_buffer_view{
                  .buffer_location = ia_address + offsetof(blocks_ia_buffer, cube_vertices),
                  .size_in_bytes = sizeof(blocks_ia_buffer::cube_vertices),
                  .stride_in_bytes = sizeof(world::block_vertex),
               });

         command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::cube_indices) / 2,
                                             view.dynamic_boxes.count, 0, 0, 0);
      }

      if (view.dynamic_ramps.count > 0) {
         command_list.set_graphics_srv(rs::block::instances_index_srv,
                                       view.dynamic_ramps.instances);
         command_list.set_graphics_srv(rs::block::instances_srv,
                                       _device.get_gpu_virtual_address(
                                          _dynamic_blocks->ramps_instance_data.get()));

         command_list.ia_set_index_buffer({
            .buffer_location = ia_address + offsetof(blocks_ia_buffer, ramp_indices),
            .size_in_bytes = sizeof(blocks_ia_buffer::ramp_indices),
         });
         command_list.ia_set_vertex_buffers(
            0, gpu::vertex_buffer_view{
                  .buffer_location = ia_address + offsetof(blocks_ia_buffer, ramp_vertices),
                  .size_in_bytes = sizeof(blocks_ia_buffer::ramp_vertices),
                  .stride_in_bytes = sizeof(world::block_vertex),
               });

         command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::ramp_indices) / 2,
                                             view.dynamic_ramps.count, 0, 0, 0);
      }

      if (view.dynamic_hemispheres.count > 0) {
         command_list.set_graphics_srv(rs::block::instances_index_srv,
                                       view.dynamic_hemispheres.instances);
         command_list
            .set_graphics_srv(rs::block::instances_srv,
                              _device.get_gpu_virtual_address(
                                 _dynamic_blocks->hemispheres_instance_data.get()));

         command_list.ia_set_index_buffer({
            .buffer_location = ia_address + offsetof(blocks_ia_buffer, hemisphere_indices),
            .size_in_bytes = sizeof(blocks_ia_buffer::hemisphere_indices),
         });
         command_list.ia_set_vertex_buffers(
            0, gpu::vertex_buffer_view{
                  .buffer_location =
                     ia_address + offsetof(blocks_ia_buffer, hemisphere_vertices),
                  .size_in_bytes = sizeof(blocks_ia_buffer::hemisphere_vertices),
                  .stride_in_bytes = sizeof(world::block_vertex),
               });

         command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::hemisphere_indices) / 2,
                                             view.dynamic_hemispheres.count, 0, 0, 0);
      }

      if (view.dynamic_pyramids.count > 0) {
         command_list.set_graphics_srv(rs::block::instances_index_srv,
                                       view.dynamic_pyramids.instances);
         command_list
            .set_graphics_srv(rs::block::instances_srv,
                              _device.get_gpu_virtual_address(
                                 _dynamic_blocks->pyramids_instance_data.get()));

         command_list.ia_set_index_buffer({
            .buffer_location = ia_address + offsetof(blocks_ia_buffer, pyramid_indices),
            .size_in_bytes = sizeof(blocks_ia_buffer::pyramid_indices),
         });
         command_list.ia_set_vertex_buffers(
            0, gpu::vertex_buffer_view{
                  .buffer_location =
                     ia_address + offsetof(blocks_ia_buffer, pyramid_vertices),
                  .size_in_bytes = sizeof(blocks_ia_buffer::pyramid_vertices),
                  .stride_in_bytes = sizeof(world::block_vertex),
               });

         command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::pyramid_indices) / 2,
                                             view.dynamic_pyramids.count, 0, 0, 0);
      }

      if (view.dynamic_quads.count > 0) {
         command_list.set_pipeline_state(select_pipeline_quads(draw, pipelines));

         command_list.set_graphics_srv(rs::block::instances_index_srv,
                                       view.dynamic_quads.instances);
         command_list.set_graphics_srv(rs::block::instances_srv,
                                       _device.get_gpu_virtual_address(
                                          _dynamic_blocks->quads_instance_data.get()));

         command_list.draw_instanced(view.dynamic_quads.count * 6, 1, 0, 0);
      }
   }

   if (view.custom.count > 0) {
      command_list.set_graphics_root_signature(root_signatures.block_custom_mesh.get());

      command_list.set_graphics_srv(rs::block_custom_mesh::instances_srv,
                                    _device.get_gpu_virtual_address(
                                       _custom_instance_data.get()));
      command_list.set_graphics_cbv(rs::block_custom_mesh::frame_cbv,
                                    frame_constant_buffer_view);
      command_list.set_graphics_cbv(rs::block_custom_mesh::lights_cbv,
                                    lights_constant_buffer_view);
      command_list.set_graphics_cbv(rs::block_custom_mesh::materials_cbv,
                                    _device.get_gpu_virtual_address(
                                       _blocks_materials_buffer.get()));

      command_list.set_pipeline_state(select_pipeline_custom_mesh(draw, pipelines));

      command_list.ia_set_index_buffer(
         {.buffer_location = _device.get_gpu_virtual_address(_custom_mesh_buffer.get()),
          .size_in_bytes = _custom_mesh_buffer_capacity});
      command_list.ia_set_vertex_buffers(0, gpu::vertex_buffer_view{

                                               .buffer_location = _device.get_gpu_virtual_address(
                                                  _custom_mesh_buffer.get()),
                                               .size_in_bytes = _custom_mesh_buffer_capacity,
                                               .stride_in_bytes =
                                                  sizeof(world::block_vertex)});

      command_list.execute_indirect(_custom_mesh_command_signature.get(),
                                    view.custom.count, view.custom.draw_buffer,
                                    view.custom.draw_buffer_offset);
   }

   [[unlikely]] if (_dynamic_blocks) {
      if (view.dynamic_custom.count > 0) {
         command_list.set_graphics_root_signature(
            root_signatures.block_custom_mesh.get());

         command_list.set_graphics_srv(rs::block_custom_mesh::instances_srv,
                                       _device.get_gpu_virtual_address(
                                          _dynamic_blocks->custom_instance_data.get()));
         command_list.set_graphics_cbv(rs::block_custom_mesh::frame_cbv,
                                       frame_constant_buffer_view);
         command_list.set_graphics_cbv(rs::block_custom_mesh::lights_cbv,
                                       lights_constant_buffer_view);
         command_list.set_graphics_cbv(rs::block_custom_mesh::materials_cbv,
                                       _device.get_gpu_virtual_address(
                                          _blocks_materials_buffer.get()));

         command_list.set_pipeline_state(select_pipeline_custom_mesh(draw, pipelines));

         command_list.ia_set_index_buffer(
            {.buffer_location =
                _device.get_gpu_virtual_address(_custom_mesh_buffer.get()),
             .size_in_bytes = _custom_mesh_buffer_capacity});
         command_list.ia_set_vertex_buffers(
            0, gpu::vertex_buffer_view{

                  .buffer_location =
                     _device.get_gpu_virtual_address(_custom_mesh_buffer.get()),
                  .size_in_bytes = _custom_mesh_buffer_capacity,
                  .stride_in_bytes = sizeof(world::block_vertex)});

         command_list.execute_indirect(_custom_mesh_command_signature.get(),
                                       view.dynamic_custom.count,
                                       view.dynamic_custom.draw_buffer,
                                       view.dynamic_custom.draw_buffer_offset);
      }
   }
}

auto blocks::get_block_mesh(const world::block_type type) const noexcept -> mesh
{
   const gpu_virtual_address ia_address =
      _device.get_gpu_virtual_address(_blocks_ia_buffer.get());

   switch (type) {
   case world::block_type::box:
      return {
         .index_buffer_view =
            {
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, cube_indices),
               .size_in_bytes = sizeof(blocks_ia_buffer::cube_indices),
            },
         .vertex_buffer_view =
            {
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, cube_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::cube_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            },
         .index_count = sizeof(blocks_ia_buffer::cube_indices) / 2,
      };
   case world::block_type::ramp:
      return {
         .index_buffer_view =
            {
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, ramp_indices),
               .size_in_bytes = sizeof(blocks_ia_buffer::ramp_indices),
            },
         .vertex_buffer_view =
            {
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, ramp_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::ramp_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            },
         .index_count = sizeof(blocks_ia_buffer::ramp_indices) / 2,
      };
   case world::block_type::quad:
      return {};
   case world::block_type::custom:
      return {};
   case world::block_type::hemisphere:
      return {
         .index_buffer_view =
            {
               .buffer_location =
                  ia_address + offsetof(blocks_ia_buffer, hemisphere_indices),
               .size_in_bytes = sizeof(blocks_ia_buffer::hemisphere_indices),
            },
         .vertex_buffer_view =
            {
               .buffer_location =
                  ia_address + offsetof(blocks_ia_buffer, hemisphere_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::hemisphere_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            },
         .index_count = sizeof(blocks_ia_buffer::hemisphere_indices) / 2,
      };
   case world::block_type::pyramid:
      return {
         .index_buffer_view =
            {
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, pyramid_indices),
               .size_in_bytes = sizeof(blocks_ia_buffer::pyramid_indices),
            },
         .vertex_buffer_view =
            {
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, pyramid_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::pyramid_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            },
         .index_count = sizeof(blocks_ia_buffer::pyramid_indices) / 2,
      };
   case world::block_type::terrain_cut_box:
      return {
         .index_buffer_view =
            {
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, cube_indices),
               .size_in_bytes = sizeof(blocks_ia_buffer::cube_indices),
            },
         .vertex_buffer_view =
            {
               .buffer_location = ia_address + offsetof(blocks_ia_buffer, cube_vertices),
               .size_in_bytes = sizeof(blocks_ia_buffer::cube_vertices),
               .stride_in_bytes = sizeof(world::block_vertex),
            },
         .index_count = sizeof(blocks_ia_buffer::cube_indices) / 2,
      };
   }

   std::unreachable();
}

auto blocks::get_block_mesh(const world::block_custom_mesh_handle handle) const noexcept
   -> mesh
{
   const uint32 index = world::blocks_custom_mesh_library::unpack_pool_index(handle);

   if (index >= _custom_meshes.size()) return {};
   if (not _custom_meshes[index].ia_allocation) return {};

   const gpu_virtual_address ia_address =
      _device.get_gpu_virtual_address(_custom_mesh_buffer.get());

   const custom_mesh& mesh = _custom_meshes[index];

   return {
      .index_buffer_view =
         {
            .buffer_location = ia_address + mesh.start_index_location * sizeof(uint16),
            .size_in_bytes = static_cast<uint32>(mesh.index_count * sizeof(uint16)),
         },
      .vertex_buffer_view =
         {
            .buffer_location = ia_address + mesh.base_vertex_location *
                                               sizeof(world::block_vertex),
            .size_in_bytes =
               static_cast<uint32>(mesh.vertex_count * sizeof(world::block_vertex)),
            .stride_in_bytes = sizeof(world::block_vertex),
         },
      .index_count = mesh.index_count,
   };
}

void blocks::process_updated_textures(gpu::copy_command_list& command_list,
                                      const updated_textures& updated)
{
   const gpu_virtual_address materials_gpu_base =
      _device.get_gpu_virtual_address(_blocks_materials_buffer.get());

   for (uint32 material_index = 0; material_index < _materials.size(); ++material_index) {
      material& material = _materials[material_index];
      const gpu_virtual_address material_gpu =
         materials_gpu_base + material_index * sizeof(block_material);

      if (auto new_texture = updated.check(material.diffuse_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.diffuse_map.texture = std::move(new_texture);
         material.diffuse_map.load_token = nullptr;

         command_list.write_buffer_immediate(
            material_gpu + offsetof(block_material, diffuse_map_index),
            material.diffuse_map.texture->srv_srgb.index);
      }

      if (auto new_texture = updated.check(material.normal_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.normal_map.texture = std::move(new_texture);
         material.normal_map.load_token = nullptr;

         command_list.write_buffer_immediate(material_gpu + offsetof(block_material,
                                                                     normal_map_index),
                                             material.normal_map.texture->srv.index);
      }

      if (auto new_texture = updated.check(material.detail_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.detail_map.texture = std::move(new_texture);
         material.detail_map.load_token = nullptr;

         command_list.write_buffer_immediate(material_gpu + offsetof(block_material,
                                                                     detail_map_index),
                                             material.detail_map.texture->srv.index);
      }

      if (auto new_texture = updated.check(material.env_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::cube) {
         material.env_map.texture = std::move(new_texture);
         material.env_map.load_token = nullptr;

         command_list.write_buffer_immediate(material_gpu + offsetof(block_material,
                                                                     env_map_index),
                                             material.env_map.texture->srv_srgb.index);
      }
   }

   [[unlikely]] if (_dynamic_blocks) {
      _dynamic_blocks->process_updated_textures(updated);
   }
}

void blocks::process_updated_textures_copy(dynamic_buffer_allocator& allocator,
                                           gpu::copy_command_list& command_list,
                                           const updated_textures& updated)
{
   for (uint32 material_index = 0; material_index < _materials.size(); ++material_index) {
      material& material = _materials[material_index];

      bool update = false;

      if (auto new_texture = updated.check(material.diffuse_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.diffuse_map.texture = std::move(new_texture);
         material.diffuse_map.load_token = nullptr;

         update = true;
      }

      if (auto new_texture = updated.check(material.normal_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.normal_map.texture = std::move(new_texture);
         material.normal_map.load_token = nullptr;

         update = true;
      }

      if (auto new_texture = updated.check(material.detail_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.detail_map.texture = std::move(new_texture);
         material.detail_map.load_token = nullptr;

         update = true;
      }

      if (auto new_texture = updated.check(material.env_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::cube) {
         material.env_map.texture = std::move(new_texture);
         material.env_map.load_token = nullptr;

         update = true;
      }

      if (update) {
         auto upload_allocation = allocator.allocate_and_copy(block_material{
            .flags = material_flags(material),
            .diffuse_map_index = material.diffuse_map.texture->srv_srgb.index,
            .normal_map_index = material.normal_map.texture->srv.index,
            .detail_map_index = material.detail_map.texture->srv.index,
            .specular_color = material.specular_lighting ? material.specular_color
                                                         : float3{0.0f, 0.0f, 0.0f},
            .env_map_index = material.env_map.texture->srv_srgb.index,
            .detail_scale = {std::max(material.detail_tiling[0], uint8{1}) * 1.0f,
                             std::max(material.detail_tiling[1], uint8{1}) * 1.0f},
            .env_color = not material.env_map.name.empty() ? material.specular_color

                                                           : float3{1.0f, 1.0f, 1.0f},
         });

         command_list.copy_buffer_region(_blocks_materials_buffer.get(),
                                         material_index * sizeof(block_material),
                                         upload_allocation.resource,
                                         upload_allocation.offset,
                                         sizeof(block_material));
      }
   }

   [[unlikely]] if (_dynamic_blocks) {
      _dynamic_blocks->process_updated_textures(updated);
   }
}

void blocks::dynamic_blocks::update(const world::entity_group& entity_group,
                                    gpu::device& device,
                                    gpu::copy_command_list& command_list,
                                    dynamic_buffer_allocator& dynamic_buffer_allocator,
                                    texture_manager& texture_manager)
{
   auto& blocks = entity_group.blocks;

   if (not blocks.boxes.empty()) {
      if (boxes_instance_data_capacity < blocks.boxes.size()) {
         boxes_instance_data_capacity = blocks.boxes.size();
         boxes_instance_data = {
            device.create_buffer({.size = boxes_instance_data_capacity *
                                          sizeof(block_instance_description),
                                  .debug_name = "World Dynamic Blocks (Boxes)"},
                                 gpu::heap_type::default_),
            device};
      }

      boxes_bbox.clear();
      boxes_bbox.reserve(blocks.boxes.size());

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(blocks.boxes.size() *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = 0; block_index < blocks.boxes.size(); ++block_index) {
         const world::block_description_box& block = blocks.boxes[block_index];

         const quaternion block_rotation = entity_group.rotation * block.rotation;
         const float3 block_positionWS =
            entity_group.rotation * block.position + entity_group.position;

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block_rotation);
         const float4x4 world_from_local = rotation * scale;

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block_positionWS;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);

         boxes_bbox.push_back(entity_group.rotation * world::get_bounding_box(block) +
                              entity_group.position);
      }

      command_list.copy_buffer_region(boxes_instance_data.get(), 0,
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      blocks.boxes.size() *
                                         sizeof(block_instance_description));
   }

   if (not blocks.ramps.empty()) {
      if (ramps_instance_data_capacity < blocks.ramps.size()) {
         ramps_instance_data_capacity = blocks.ramps.size();
         ramps_instance_data = {
            device.create_buffer({.size = ramps_instance_data_capacity *
                                          sizeof(block_instance_description),
                                  .debug_name = "World Dynamic Blocks (Ramps)"},
                                 gpu::heap_type::default_),
            device};
      }

      ramps_bbox.clear();
      ramps_bbox.reserve(blocks.ramps.size());

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(blocks.ramps.size() *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = 0; block_index < blocks.ramps.size(); ++block_index) {
         const world::block_description_ramp& block = blocks.ramps[block_index];

         const quaternion block_rotation = entity_group.rotation * block.rotation;
         const float3 block_positionWS =
            entity_group.rotation * block.position + entity_group.position;

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block_rotation);
         const float4x4 world_from_local = rotation * scale;

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block_positionWS;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);

         ramps_bbox.push_back(entity_group.rotation * world::get_bounding_box(block) +
                              entity_group.position);
      }

      command_list.copy_buffer_region(ramps_instance_data.get(), 0,
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      blocks.ramps.size() *
                                         sizeof(block_instance_description));
   }

   if (not blocks.quads.empty()) {
      if (quads_instance_data_capacity < blocks.quads.size()) {
         quads_instance_data_capacity = blocks.quads.size();
         quads_instance_data = {
            device.create_buffer({.size = quads_instance_data_capacity *
                                          sizeof(block_quad_description),
                                  .debug_name = "World Dynamic Blocks (Quads)"},
                                 gpu::heap_type::default_),
            device};
      }

      quads_bbox.clear();
      quads_bbox.reserve(blocks.quads.size());

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(blocks.quads.size() *
                                           sizeof(block_quad_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = 0; block_index < blocks.quads.size(); ++block_index) {
         const world::block_description_quad& block = blocks.quads[block_index];

         block_quad_description description;

         description.positionWS = {
            entity_group.rotation * block.vertices[0] + entity_group.position,
            entity_group.rotation * block.vertices[1] + entity_group.position,
            entity_group.rotation * block.vertices[2] + entity_group.position,
            entity_group.rotation * block.vertices[3] + entity_group.position,
         };
         description.surface = {
            .material_index = block.surface_materials[0],
            .texture_mode = static_cast<uint32>(block.surface_texture_mode[0]),
            .scaleX = static_cast<uint32>(block.surface_texture_scale[0][0] + 7),
            .scaleY = static_cast<uint32>(block.surface_texture_scale[0][1] + 7),
            .rotation = static_cast<uint32>(block.surface_texture_rotation[0]),
            .offsetX = block.surface_texture_offset[0][0],
            .offsetY = block.surface_texture_offset[0][1],
            .quad_split = static_cast<uint32>(block.quad_split),
         };

         description.normalWS[0] = normalize(
            cross(description.positionWS[world::block_quad_triangles[0][1]] -
                     description.positionWS[world::block_quad_triangles[0][0]],
                  description.positionWS[world::block_quad_triangles[0][2]] -
                     description.positionWS[world::block_quad_triangles[0][0]]));
         description.normalWS[1] = normalize(
            cross(description.positionWS[world::block_quad_triangles[1][1]] -
                     description.positionWS[world::block_quad_triangles[1][0]],
                  description.positionWS[world::block_quad_triangles[1][2]] -
                     description.positionWS[world::block_quad_triangles[1][0]]));

         std::memcpy(upload_ptr, &description, sizeof(block_quad_description));

         upload_ptr += sizeof(block_quad_description);

         quads_bbox.push_back(entity_group.rotation * world::get_bounding_box(block) +
                              entity_group.position);
      }

      command_list.copy_buffer_region(quads_instance_data.get(), 0,
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      blocks.quads.size() *
                                         sizeof(block_quad_description));
   }

   if (not blocks.hemispheres.empty()) {
      if (hemispheres_instance_data_capacity < blocks.hemispheres.size()) {
         hemispheres_instance_data_capacity = blocks.hemispheres.size();
         hemispheres_instance_data =
            {device.create_buffer({.size = hemispheres_instance_data_capacity *
                                           sizeof(block_instance_description),
                                   .debug_name =
                                      "World Dynamic Blocks (Hemispheres)"},
                                  gpu::heap_type::default_),
             device};
      }

      hemispheres_bbox.clear();
      hemispheres_bbox.reserve(blocks.hemispheres.size());

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(blocks.hemispheres.size() *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = 0; block_index < blocks.hemispheres.size();
           ++block_index) {
         const world::block_description_hemisphere& block =
            blocks.hemispheres[block_index];

         const quaternion block_rotation = entity_group.rotation * block.rotation;
         const float3 block_positionWS =
            entity_group.rotation * block.position + entity_group.position;

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block_rotation);
         const float4x4 world_from_local = rotation * scale;

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block_positionWS;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);

         hemispheres_bbox.push_back(entity_group.rotation *
                                       world::get_bounding_box(block) +
                                    entity_group.position);
      }

      command_list.copy_buffer_region(hemispheres_instance_data.get(), 0,
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      blocks.hemispheres.size() *
                                         sizeof(block_instance_description));
   }

   if (not blocks.pyramids.empty()) {
      if (pyramids_instance_data_capacity < blocks.pyramids.size()) {
         pyramids_instance_data_capacity = blocks.pyramids.size();
         pyramids_instance_data =
            {device.create_buffer({.size = pyramids_instance_data_capacity *
                                           sizeof(block_instance_description),
                                   .debug_name =
                                      "World Dynamic Blocks (pyramids)"},
                                  gpu::heap_type::default_),
             device};
      }

      pyramids_bbox.clear();
      pyramids_bbox.reserve(blocks.pyramids.size());

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(blocks.pyramids.size() *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = 0; block_index < blocks.pyramids.size(); ++block_index) {
         const world::block_description_pyramid& block = blocks.pyramids[block_index];

         const quaternion block_rotation = entity_group.rotation * block.rotation;
         const float3 block_positionWS =
            entity_group.rotation * block.position + entity_group.position;

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block_rotation);
         const float4x4 world_from_local = rotation * scale;

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block_positionWS;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);

         pyramids_bbox.push_back(entity_group.rotation * world::get_bounding_box(block) +
                                 entity_group.position);
      }

      command_list.copy_buffer_region(pyramids_instance_data.get(), 0,
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      blocks.pyramids.size() *
                                         sizeof(block_instance_description));
   }

   if (not blocks.custom.description.empty()) {
      if (custom_instance_data_capacity < blocks.custom.description.size()) {
         custom_instance_data_capacity = blocks.custom.description.size();
         custom_instance_data =
            {device.create_buffer({.size = custom_instance_data_capacity *
                                           sizeof(block_instance_description),
                                   .debug_name =
                                      "World Dynamic Blocks (Cylinders)"},
                                  gpu::heap_type::default_),
             device};
      }

      custom_bbox.clear();
      custom_bbox.reserve(blocks.custom.description.size());

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(blocks.custom.description.size() *
                                           sizeof(block_instance_description));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 block_index = 0;
           block_index < blocks.custom.description.size(); ++block_index) {
         const world::block_description_custom& block =
            blocks.custom.description[block_index];

         const quaternion block_rotation = entity_group.rotation * block.rotation;
         const float3 block_positionWS =
            entity_group.rotation * block.position + entity_group.position;

         const float4x4 world_from_local = to_matrix(block_rotation);

         block_instance_description description;

         description.adjugate_world_from_local = adjugate(world_from_local);
         description.world_from_local[0] = {world_from_local[0].x,
                                            world_from_local[0].y,
                                            world_from_local[0].z};
         description.world_from_local[1] = {world_from_local[1].x,
                                            world_from_local[1].y,
                                            world_from_local[1].z};
         description.world_from_local[2] = {world_from_local[2].x,
                                            world_from_local[2].y,
                                            world_from_local[2].z};
         description.world_from_local[3] = block_positionWS;

         const quaternion local_from_world = conjugate(block.rotation);

         description.local_from_world_xyz = {local_from_world.x,
                                             local_from_world.y,
                                             local_from_world.z};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
               .local_from_world_w_sign = local_from_world.w < 0.0f,
            };
         }

         std::memcpy(upload_ptr, &description, sizeof(block_instance_description));

         upload_ptr += sizeof(block_instance_description);

         custom_bbox.push_back(entity_group.rotation * world::get_bounding_box(block) +
                               entity_group.position);
      }

      command_list.copy_buffer_region(custom_instance_data.get(), 0,
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      blocks.custom.description.size() *
                                         sizeof(block_instance_description));
   }

   if (not materials_buffer) {
      materials_buffer = {device.create_buffer({.size = sizeof(block_material) *
                                                        world::max_block_materials,
                                                .debug_name =
                                                   "Dynamic Blocks Materials"},
                                               gpu::heap_type::default_),
                          device};

      for (material& material : materials) {
         material.diffuse_map.texture = texture_manager.null_diffuse_map();
         material.normal_map.texture = texture_manager.null_normal_map();
         material.detail_map.texture = texture_manager.null_detail_map();
         material.env_map.texture = texture_manager.null_cube_map();
      }
   }

   if (not blocks.materials.empty()) {
      const std::size_t material_count =
         std::min(blocks.materials.size(), world::max_block_materials);

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(material_count * sizeof(block_material));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 material_index = 0; material_index < material_count; ++material_index) {
         const world::block_material& world_material =
            blocks.materials[material_index];
         material& material = materials[material_index];

         material.diffuse_map.update(world_material.diffuse_map, texture_manager,
                                     texture_manager.null_diffuse_map());
         material.normal_map.update(world_material.normal_map, texture_manager,
                                    texture_manager.null_normal_map());
         material.detail_map.update(world_material.detail_map, texture_manager,
                                    texture_manager.null_detail_map());
         material.env_map.update(world_material.env_map, texture_manager,
                                 texture_manager.null_cube_map());

         material.detail_tiling = world_material.detail_tiling;
         material.tile_normal_map = world_material.tile_normal_map;
         material.specular_lighting = world_material.specular_lighting;

         material.specular_color = world_material.specular_color;

         const block_material constants{
            .flags = material_flags(material),
            .diffuse_map_index = material.diffuse_map.texture->srv_srgb.index,
            .normal_map_index = material.normal_map.texture->srv.index,
            .detail_map_index = material.detail_map.texture->srv.index,
            .specular_color = material.specular_lighting ? material.specular_color
                                                         : float3{0.0f, 0.0f, 0.0f},
            .env_map_index = material.env_map.texture->srv_srgb.index,
            .detail_scale = {std::max(material.detail_tiling[0], uint8{1}) * 1.0f,
                             std::max(material.detail_tiling[1], uint8{1}) * 1.0f},
            .env_color = not material.env_map.name.empty() ? material.specular_color
                                                           : float3{1.0f, 1.0f, 1.0f},
         };

         std::memcpy(upload_ptr, &constants, sizeof(block_material));

         upload_ptr += sizeof(block_material);
      }

      command_list.copy_buffer_region(materials_buffer.get(), 0,
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      material_count * sizeof(block_material));
   }
}

void blocks::dynamic_blocks::process_updated_textures(const updated_textures& updated)
{
   for (material& material : materials) {
      if (auto new_texture = updated.check(material.diffuse_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.diffuse_map.texture = std::move(new_texture);
         material.diffuse_map.load_token = nullptr;
      }

      if (auto new_texture = updated.check(material.normal_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.normal_map.texture = std::move(new_texture);
         material.normal_map.load_token = nullptr;
      }

      if (auto new_texture = updated.check(material.detail_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.detail_map.texture = std::move(new_texture);
         material.detail_map.load_token = nullptr;
      }

      if (auto new_texture = updated.check(material.env_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::cube) {
         material.env_map.texture = std::move(new_texture);
         material.env_map.load_token = nullptr;
      }
   }
}

auto blocks::dynamic_blocks::bbox_soa::size() const noexcept -> std::size_t
{
   return min_x.size();
}

void blocks::dynamic_blocks::bbox_soa::push_back(const math::bounding_box& bbox) noexcept
{
   min_x.push_back(bbox.min.x);
   min_y.push_back(bbox.min.y);
   min_z.push_back(bbox.min.z);
   max_x.push_back(bbox.max.x);
   max_y.push_back(bbox.max.y);
   max_z.push_back(bbox.max.z);
}

void blocks::dynamic_blocks::bbox_soa::reserve(std::size_t size) noexcept
{
   min_x.reserve(size);
   min_y.reserve(size);
   min_z.reserve(size);
   max_x.reserve(size);
   max_y.reserve(size);
   max_z.reserve(size);
}

void blocks::dynamic_blocks::bbox_soa::clear() noexcept
{
   min_x.clear();
   min_y.clear();
   min_z.clear();
   max_x.clear();
   max_y.clear();
   max_z.clear();
}

void blocks::material::texture::update(const std::string_view new_name,
                                       texture_manager& texture_manager,
                                       const std::shared_ptr<const world_texture>& default_texture)
{
   if (string::iequals(name, new_name)) return;

   name = lowercase_string{new_name};
   texture = texture_manager.at_or(name, world_texture_dimension::_2d, nullptr);
   load_token = nullptr;

   if (not texture) {
      texture = default_texture;
      load_token = texture_manager.acquire_load_token(name);
   }
}
}