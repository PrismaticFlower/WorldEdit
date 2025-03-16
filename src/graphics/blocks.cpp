#include "blocks.hpp"
#include "cull_objects.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

namespace we::graphics {

namespace {

struct block_instance_transform {
   float4x4 world_from_object;
   float3x3 adjugate_world_from_object;
};

struct block_vertex {
   float3 position;
   float3 normal;
   float2 texcoords;
};

struct blocks_ia_buffer {
   std::array<block_vertex, 24> cube_vertices = {{
      {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
      {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
      {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
      {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
      {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
      {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
      {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
      {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
      {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
      {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
      {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
      {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
      {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
      {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
   }};

   std::array<uint16, 36> cube_indices = {{
      0, 1,  2, 3, 4,  5, 6, 7,  8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
      0, 18, 1, 3, 19, 4, 6, 20, 7, 9, 21, 10, 12, 22, 13, 15, 23, 16,
   }};
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
   if (_boxes_instance_data_capacity < blocks.cubes.size()) {
      const gpu::unique_resource_handle old_boxes_instance_data =
         std::move(_boxes_instance_data);
      const uint64 old_boxes_instance_data_capacity = _boxes_instance_data_capacity;

      _boxes_instance_data_capacity = blocks.cubes.size() * 16180 / 10000;
      _boxes_instance_data =
         {_device.create_buffer({.size = _boxes_instance_data_capacity *
                                         sizeof(block_instance_transform),
                                 .debug_name = "World blocks (Boxes)"},
                                gpu::heap_type::default_),
          _device.direct_queue};

      if (old_boxes_instance_data_capacity != 0) {
         command_list.copy_buffer_region(_boxes_instance_data.get(), 0,
                                         old_boxes_instance_data.get(), 0,
                                         old_boxes_instance_data_capacity *
                                            sizeof(block_instance_transform));
      }
   }

   for (const world::blocks_dirty_range& range : blocks.cubes.dirty) {
      const uint32 range_size = range.end - range.begin;

      const dynamic_buffer_allocator::allocation& upload_allocation =
         dynamic_buffer_allocator.allocate(range_size * sizeof(block_instance_transform));
      std::byte* upload_ptr = upload_allocation.cpu_address;

      for (uint32 i = range.begin; i < range.end; ++i) {
         const world::block_description_cube& block = blocks.cubes.description[i];

         const float4x4 scale = {
            {block.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, block.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, block.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(block.rotation);

         block_instance_transform transform;

         transform.world_from_object = rotation * scale;
         transform.adjugate_world_from_object = float3x3(transform.world_from_object);
         transform.world_from_object[3] = {block.position, 1.0f};

         std::memcpy(upload_ptr, &transform, sizeof(block_instance_transform));

         upload_ptr += sizeof(block_instance_transform);
      }

      command_list.copy_buffer_region(_boxes_instance_data.get(),
                                      range.begin * sizeof(block_instance_transform),
                                      upload_allocation.resource,
                                      upload_allocation.offset,
                                      range_size * sizeof(block_instance_transform));
   }

   (void)texture_manager;
}

auto blocks::prepare_view(blocks_draw draw, const world::blocks& blocks,
                          const frustum& view_frustum,
                          dynamic_buffer_allocator& dynamic_buffer_allocator) -> view
{
   view view;

   _TEMP_culling_storage.clear();

   _TEMP_culling_storage.reserve(blocks.cubes.size());

   if (draw == blocks_draw::shadow) {
      cull_objects_shadow_cascade_avx2(view_frustum, blocks.cubes.bbox.min_x,
                                       blocks.cubes.bbox.min_y,
                                       blocks.cubes.bbox.min_z,
                                       blocks.cubes.bbox.max_x,
                                       blocks.cubes.bbox.max_y,
                                       blocks.cubes.bbox.max_z, _TEMP_culling_storage);
   }
   else {
      cull_objects_avx2(view_frustum, blocks.cubes.bbox.min_x,
                        blocks.cubes.bbox.min_y, blocks.cubes.bbox.min_z,
                        blocks.cubes.bbox.max_x, blocks.cubes.bbox.max_y,
                        blocks.cubes.bbox.max_z, _TEMP_culling_storage);
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

      view.cube_instances_count = static_cast<uint32>(_TEMP_culling_storage.size());
      view.cube_instances = instance_index_allocation.gpu_address;
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

   if (view.cube_instances_count > 0) {
      command_list.set_graphics_srv(rs::block::instances_index_srv, view.cube_instances);
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
               .stride_in_bytes = sizeof(block_vertex),
            });

      command_list.draw_indexed_instanced(sizeof(blocks_ia_buffer::cube_indices) / 2,
                                          view.cube_instances_count, 0, 0, 0);
   }
}

}