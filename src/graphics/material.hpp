#pragma once

#include "assets/msh/material.hpp"
#include "copy_command_list_pool.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "pipeline_library.hpp"
#include "texture_manager.hpp"

namespace we::graphics {

struct material {
   material(const assets::msh::material& material, gpu::device& device,
            copy_command_list_pool& copy_command_list_pool,
            texture_manager& texture_manager);

   void init_resources(gpu::device& device,
                       copy_command_list_pool& copy_command_list_pool);

   void process_updated_textures(gpu::graphics_command_list& command_list,
                                 const updated_textures& updated, gpu::device& device);

   material_pipeline_flags flags = material_pipeline_flags::none;
   gpu_virtual_address constant_buffer_view;

   struct textures {
      std::shared_ptr<const world_texture> diffuse_map;
      std::shared_ptr<const world_texture> normal_map;
   };

   struct texture_names {
      lowercase_string diffuse_map;
      lowercase_string normal_map;
   };

   gpu::unique_resource_handle constant_buffer;

   textures textures;
   texture_names texture_names;

   float3 specular_color;

   assets::msh::material_flags msh_flags = assets::msh::material_flags::none;
};

}
