#pragma once

#include "assets/msh/material.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "pipeline_library.hpp"
#include "texture_manager.hpp"

namespace we::graphics {

struct material {
   material(const assets::msh::material& material, gpu::device& gpu_device,
            texture_manager& texture_manager);

   void init_resources(gpu::device& gpu_device);

   void process_updated_textures(gpu::graphics_command_list& command_list,
                                 const updated_textures& updated);

   material_pipeline_flags flags = material_pipeline_flags::none;
   gpu::buffer constant_buffer;
   gpu::resource_view_set constant_buffer_view;

   struct textures {
      std::shared_ptr<const world_texture> diffuse_map;
      std::shared_ptr<const world_texture> normal_map;
   };

   struct texture_names {
      lowercase_string diffuse_map;
      lowercase_string normal_map;
   };

   textures textures;
   texture_names texture_names;

   float3 specular_color;

   assets::msh::material_flags msh_flags = assets::msh::material_flags::none;
};

}
