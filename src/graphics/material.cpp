#include "material.hpp"

namespace sk::graphics {

material::material(const assets::msh::material& material,
                   gpu::device& gpu_device, texture_manager& texture_manager)
{
   (void)material, gpu_device, texture_manager; // TODO: Create material.
}

}
