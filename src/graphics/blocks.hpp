#pragma once

#include "copy_command_list_pool.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "geometric_shapes.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "math/frustum.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "texture_manager.hpp"
#include "types.hpp"
#include "world/world.hpp"

#include <vector>

namespace we::graphics {

enum class blocks_draw { depth_prepass, main, shadow };

struct blocks {
   struct view {
      uint32 cube_instances_count = 0;
      gpu_virtual_address cube_instances = 0;
   };

   blocks(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
          dynamic_buffer_allocator& dynamic_buffer_allocator);

   void update(const world::blocks& blocks, gpu::copy_command_list& command_list,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager);

   auto prepare_view(blocks_draw draw, const world::blocks& blocks,
                     const frustum& view_frustum,
                     dynamic_buffer_allocator& dynamic_buffer_allocator) -> view;

   void draw(blocks_draw draw, const view& view,
             gpu_virtual_address frame_constant_buffer_view,
             gpu_virtual_address lights_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines) const;

   void process_updated_texture(const updated_textures& updated);

private:
   gpu::device& _device;

   gpu::unique_resource_handle _blocks_ia_buffer;

   gpu::unique_resource_handle _boxes_instance_data;
   uint64 _boxes_instance_data_capacity = 0;

   std::vector<uint16> _TEMP_culling_storage;
};

}
