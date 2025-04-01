#include "blocks.hpp"
#include "cull_objects.hpp"

#include "world/blocks/mesh_geometry.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

#include "utility/enum_bitflags.hpp"
#include "utility/string_icompare.hpp"

namespace we::graphics {

namespace {

struct surface_info {
   uint32 material_index : 8;
   uint32 texture_mode : 3;
   uint32 scaleX : 4;
   uint32 scaleY : 4;
   uint32 rotation : 2;
   uint32 offsetX : 13;
   uint32 offsetY : 13;
};

static_assert(sizeof(surface_info) == 8);

struct block_instance_description {
   float4x4 world_from_object;
   float3x3 adjugate_world_from_object;
   std::array<surface_info, 6> surfaces;
   std::array<uint32, 3> padding;
};

static_assert(sizeof(block_instance_description) == 160);

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

struct blocks_ia_buffer {
   std::array<world::block_vertex, 24> cube_vertices = world::block_cube_vertices;
   std::array<std::array<uint16, 3>, 12> cube_indices = world::block_cube_triangles;
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

}

blocks::blocks(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager)
   : _device{device}
{
   _blocks_ia_buffer = {_device.create_buffer({.size = sizeof(blocks_ia_buffer),
                                               .debug_name =
                                                  "Blocks IA Buffer"},
                                              gpu::heap_type::default_),
                        _device.direct_queue};

   _blocks_materials_buffer =
      {_device.create_buffer({.size = sizeof(block_material) * world::max_block_materials,
                              .debug_name = "Blocks Materials"},
                             gpu::heap_type::default_),
       _device.direct_queue};

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
}

void blocks::update(const world::blocks& blocks, gpu::copy_command_list& command_list,
                    dynamic_buffer_allocator& dynamic_buffer_allocator,
                    texture_manager& texture_manager)
{
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
          _device.direct_queue};

      if (old_boxes_instance_data_capacity != 0) {
         command_list.copy_buffer_region(_boxes_instance_data.get(), 0,
                                         old_boxes_instance_data.get(), 0,
                                         old_boxes_instance_data_capacity *
                                            sizeof(block_instance_description));
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

         block_instance_description description;

         description.world_from_object = rotation * scale;
         description.adjugate_world_from_object =
            float3x3(description.world_from_object);
         description.world_from_object[3] = {block.position, 1.0f};

         for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
            description.surfaces[i] = {
               .material_index = block.surface_materials[i],
               .texture_mode = static_cast<uint32>(block.surface_texture_mode[i]),
               .scaleX = static_cast<uint32>(block.surface_texture_scale[i][0] + 7),
               .scaleY = static_cast<uint32>(block.surface_texture_scale[i][1] + 7),
               .rotation = static_cast<uint32>(block.surface_texture_rotation[i]),
               .offsetX = block.surface_texture_offset[i][0],
               .offsetY = block.surface_texture_offset[i][1],
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
}

auto blocks::prepare_view(blocks_draw draw, const world::blocks& blocks,
                          const frustum& view_frustum,
                          dynamic_buffer_allocator& dynamic_buffer_allocator) -> view
{
   view view;

   _TEMP_culling_storage.clear();

   _TEMP_culling_storage.reserve(blocks.boxes.size());

   if (draw == blocks_draw::shadow) {
      cull_objects_shadow_cascade_avx2(view_frustum, blocks.boxes.bbox.min_x,
                                       blocks.boxes.bbox.min_y,
                                       blocks.boxes.bbox.min_z,
                                       blocks.boxes.bbox.max_x,
                                       blocks.boxes.bbox.max_y,
                                       blocks.boxes.bbox.max_z, _TEMP_culling_storage);
   }
   else {
      cull_objects_avx2(view_frustum, blocks.boxes.bbox.min_x,
                        blocks.boxes.bbox.min_y, blocks.boxes.bbox.min_z,
                        blocks.boxes.bbox.max_x, blocks.boxes.bbox.max_y,
                        blocks.boxes.bbox.max_z, _TEMP_culling_storage);
   }

   if (not _TEMP_culling_storage.empty()) {
      const dynamic_buffer_allocator::allocation& instance_index_allocation =
         dynamic_buffer_allocator.allocate(_TEMP_culling_storage.size() *
                                           sizeof(uint32));

      volatile uint32* next_instance =
         reinterpret_cast<uint32*>(instance_index_allocation.cpu_address);

      for (const uint32 index : _TEMP_culling_storage) {
         *next_instance = index;

         next_instance += 1;
      }

      view.box_instances_count = static_cast<uint32>(_TEMP_culling_storage.size());
      view.box_instances = instance_index_allocation.gpu_address;
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

   if (view.box_instances_count > 0) {
      command_list.set_graphics_srv(rs::block::instances_index_srv, view.box_instances);
      command_list.set_graphics_srv(rs::block::instances_srv,
                                    _device.get_gpu_virtual_address(
                                       _boxes_instance_data.get()));

      const gpu_virtual_address ia_address =
         _device.get_gpu_virtual_address(_blocks_ia_buffer.get());

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
                                          view.box_instances_count, 0, 0, 0);
   }
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

         command_list.write_buffer_immediate(
            material_gpu + offsetof(block_material, normal_map_index),
            material.normal_map.texture->srv_srgb.index);
      }

      if (auto new_texture = updated.check(material.detail_map.name);
          new_texture and new_texture->dimension == world_texture_dimension::_2d) {
         material.detail_map.texture = std::move(new_texture);
         material.detail_map.load_token = nullptr;

         command_list.write_buffer_immediate(
            material_gpu + offsetof(block_material, detail_map_index),
            material.detail_map.texture->srv_srgb.index);
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