#pragma once

#include "assets/msh/material.hpp"
#include "gpu/device.hpp"
#include "texture_manager.hpp"

#include <vector>

namespace sk::graphics {

struct material {
   material(const assets::msh::material& material, gpu::device& gpu_device,
            texture_manager& texture_manager);

   gpu::resource_view_set resource_views;

   std::vector<std::shared_ptr<gpu::texture>> textures;
   std::vector<std::string> texture_names;
};

}
