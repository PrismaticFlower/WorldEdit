#include "material.hpp"

#include <DirectXTex.h>

namespace sk::graphics {

material::material(const assets::msh::material& material,
                   gpu::device& gpu_device, texture_manager& texture_manager)
{
   texture_names.resize(1);
   texture_names[0] = material.textures[0];

   textures.resize(1);

   textures[0] = texture_manager.aquire_if(texture_names[0]);

   if (not textures[0]) textures[0] = texture_manager.null_diffuse_map();

   init_resource_views(gpu_device);
}

void material::init_resource_views(gpu::device& gpu_device)
{
   const std::array resource_view_descriptions{
      gpu::resource_view_desc{.resource = *textures[0]->resource(),
                              .view_desc = gpu::shader_resource_view_desc{
                                 .format = DirectX::MakeSRGB(DirectX::MakeTypelessUNORM(textures[0]->format())),
                                 .type_description = gpu::texture2d_srv{}}}};

   resource_views = gpu_device.create_resource_view_set(resource_view_descriptions);
}

}
