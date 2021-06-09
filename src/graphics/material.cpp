#include "material.hpp"

#include <range/v3/view.hpp>

using namespace std::literals;

namespace we::graphics {

namespace {

constexpr bool has_normalmap(const assets::msh::material& material)
{
   using assets::msh::material_flags;
   using assets::msh::rendertype;

   return are_flags_set(material.flags, material_flags::perpixel) or
          are_flags_set(material.flags, material_flags::specular) or
          material.rendertype == (rendertype::normalmap) or
          material.rendertype == (rendertype::normalmap_specular) or
          material.rendertype == (rendertype::normalmap_tiled_envmapped) or
          material.rendertype == (rendertype::normalmap_tiled) or
          material.rendertype == (rendertype::normalmap_envmapped);
}

}

material::material(const assets::msh::material& material,
                   gpu::device& gpu_device, texture_manager& texture_manager)
{
   texture_names.resize(2);
   texture_names[0] = lowercase_string{material.textures[0]};
   texture_names[1] = lowercase_string{material.textures[1]};

   textures.resize(2);

   textures[0] =
      texture_manager.at_or(texture_names[0], texture_manager.null_diffuse_map());

   if (has_normalmap(material) and not texture_names[1].empty()) {
      textures[1] = texture_manager.at_or(texture_names[1],
                                          texture_manager.null_normal_map());
   }

   if (not textures[0]) textures[0] = texture_manager.null_diffuse_map();
   if (not textures[1]) textures[1] = texture_manager.null_normal_map();

   init_resource_views(gpu_device);

   using assets::msh::material_flags;
   using gpu::material_pipeline_flags;

   if (are_flags_set(material.flags, material_flags::transparent)) {
      flags |= material_pipeline_flags::transparent;
   }

   if (are_flags_set(material.flags, material_flags ::transparent_doublesided)) {
      flags |= material_pipeline_flags::transparent;
      flags |= material_pipeline_flags::doublesided;
   }

   if (are_flags_set(material.flags, material_flags ::hardedged)) {
      flags |= material_pipeline_flags::alpha_cutout;
      flags &= ~material_pipeline_flags::transparent;
   }

   if (are_flags_set(material.flags, material_flags::additive)) {
      flags |= material_pipeline_flags::additive;
   }
}

void material::init_resource_views(gpu::device& gpu_device)
{
   // const std::array resource_view_descriptions{
   //    gpu::resource_view_desc{.resource = *textures[0]->resource(),
   //                            .view_desc =
   //                               gpu::shader_resource_view_desc{
   //                                  .format = textures[0]->format(),
   //                                  .type_description = gpu::texture2d_srv{}}},
   //
   //    gpu::resource_view_desc{.resource = *textures[1]->resource(),
   //                            .view_desc =
   //                               gpu::shader_resource_view_desc{
   //                                  .format = textures[1]->format(),
   //                                  .type_description = gpu::texture2d_srv{}}},
   // };

   // Above ICEs, workaround below...

   std::array resource_view_descriptions{
      gpu::resource_view_desc{.resource = textures[0]->resource()},

      gpu::resource_view_desc{.resource = textures[1]->resource()},
   };

   resource_view_descriptions[0].view_desc =
      gpu::shader_resource_view_desc{.format = textures[0]->format(),
                                     .type_description = gpu::texture2d_srv{}};

   resource_view_descriptions[1].view_desc =
      gpu::shader_resource_view_desc{.format = textures[1]->format(),
                                     .type_description = gpu::texture2d_srv{}};

   resource_views = gpu_device.create_resource_view_set(resource_view_descriptions);
}

void material::process_updated_texture(gpu::device& gpu_device, updated_texture updated)
{
   using namespace ranges::views;

   bool reinit_views = false;

   for (auto [name, texture] : zip(texture_names, textures)) {
      if (name != updated.name) continue;

      texture = updated.texture;
      reinit_views = true;
   }

   if (reinit_views) init_resource_views(gpu_device);
}
}
