#include "blocks.hpp"
#include "cull_objects.hpp"

#include "world/blocks/mesh_geometry.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

namespace we::graphics {

namespace {

struct surface_info {
   uint32 material_index : 8 = 0;
   uint32 scale : 8 = 0;
   uint32 rotation : 2 = 0;
   uint32 : 14;
};

static_assert(sizeof(surface_info) == 4);

struct block_instance_description {
   float4x4 world_from_object;
   float3x3 adjugate_world_from_object;
   std::array<surface_info, 6> surfaces;
   uint32 padding = 0;
};

static_assert(sizeof(block_instance_description) == 128);

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
      return pipelines.block_basic_lighting.get();
   case blocks_draw::shadow:
      return pipelines.block_shadow.get();
   }

   std::unreachable();
}

}

blocks::blocks(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
               dynamic_buffer_allocator& dynamic_buffer_allocator)
   : _device{device}
{
   _blocks_ia_buffer = {_device.create_buffer({.size = sizeof(blocks_ia_buffer),
                                               .debug_name =
                                                  "Blocks IA Buffer"},
                                              gpu::heap_type::default_),
                        _device.direct_queue};

   auto upload_data = dynamic_buffer_allocator.allocate_and_copy(blocks_ia_buffer{});

   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   command_list->copy_buffer_region(_blocks_ia_buffer.get(), 0, upload_data.resource,
                                    upload_data.offset, sizeof(blocks_ia_buffer));

   command_list->close();

   device.background_copy_queue.execute_command_lists(command_list.get());
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
            description.surfaces[i] = {.material_index = block.surface_materials[i],
                                       .scale = block.surface_texture_scale[i],
                                       .rotation = static_cast<uint32>(
                                          block.surface_texture_rotation[i])};
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

   (void)texture_manager;
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

}