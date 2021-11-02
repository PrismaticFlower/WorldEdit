#pragma once

#include "assets/msh/material.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "pipeline_library.hpp"
#include "texture_manager.hpp"

#include <vector>

namespace we::graphics {

struct material {
   material(const assets::msh::material& material, gpu::device& gpu_device,
            texture_manager& texture_manager);

   void init_resources(gpu::device& gpu_device);

   void update_constant_buffer(gpu::graphics_command_list& command_list,
                               gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void process_updated_texture(gpu::graphics_command_list& command_list,
                                gpu::dynamic_buffer_allocator& dynamic_buffer_allocator,
                                updated_texture updated);

   material_pipeline_flags flags = material_pipeline_flags::none;
   gpu::buffer constant_buffer;
   gpu::resource_view_set constant_buffer_view;

   std::vector<std::shared_ptr<const world_texture>> textures;
   std::vector<lowercase_string> texture_names;
};

}
