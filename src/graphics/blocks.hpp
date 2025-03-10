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

struct blocks {
   blocks(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
          dynamic_buffer_allocator& dynamic_buffer_allocator);

   void update(const world::blocks& blocks, gpu::copy_command_list& command_list,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager);

   void draw(const world::blocks& blocks, const frustum& view_frustum,
             gpu_virtual_address frame_constant_buffer_view,
             gpu_virtual_address lights_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines,
             dynamic_buffer_allocator& dynamic_buffer_allocator,
             const geometric_shapes& geometric_shape);

   void process_updated_texture(const updated_textures& updated);

private:
   gpu::device& _device;

   gpu::unique_resource_handle _blocks_ia_buffer;

   gpu::unique_resource_handle _boxes_instance_data;
   uint64 _boxes_instance_data_capacity = 0;

   std::vector<uint16> _TEMP_culling_storage;
};

}
