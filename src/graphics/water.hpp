#pragma once

#include "dynamic_buffer_allocator.hpp"
#include "frustum.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "texture_manager.hpp"
#include "types.hpp"
#include "world/world.hpp"

namespace we::graphics {

struct water {
   explicit water(gpu::device& device, texture_manager& texture_manager);

   void init(const world::terrain& terrain, gpu::copy_command_list& command_list,
             dynamic_buffer_allocator& dynamic_buffer_allocator);

   void draw(const frustum& view_frustum, gpu_virtual_address camera_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines,
             dynamic_buffer_allocator& dynamic_buffer_allocator);

   void process_updated_texture(gpu::copy_command_list& command_list,
                                const updated_textures& updated);

private:
   gpu::device& _device;
   texture_manager& _texture_manager;

   bool _active = false;

   uint32 _point_count;

   gpu::unique_resource_handle _water_constants_buffer;
   gpu::unique_resource_handle _point_buffer;
};

}
