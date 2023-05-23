#include "material.hpp"

using namespace std::literals;

namespace we::graphics {

namespace {

enum class shader_flags : uint32 {
   none = 0b0,
   transparent = 0b1,
   unlit = 0b10,
   specular_visibility_in_diffuse_map = 0b100,
   scrolling = 0b1000,
   static_lighting = 0b10000,
   has_normal_map = 0b100000,
   has_detail_map = 0b1000000,
   has_env_map = 0b10000000,
   tile_normal_map = 0b100000000,
};

constexpr bool marked_as_enum_bitflag(shader_flags)
{
   return true;
}

constexpr bool has_normalmap(const assets::msh::material& material)
{
   using assets::msh::material_flags;
   using assets::msh::rendertype;

   return not material.textures[1].empty() and
          (are_flags_set(material.flags, material_flags::perpixel) or
           are_flags_set(material.flags, material_flags::specular) or
           material.rendertype == (rendertype::normalmap) or
           material.rendertype == (rendertype::normalmap_specular) or
           material.rendertype == (rendertype::normalmap_tiled_envmapped) or
           material.rendertype == (rendertype::normalmap_tiled) or
           material.rendertype == (rendertype::normalmap_envmapped));
}

constexpr bool has_detail_map(const assets::msh::material& material)
{
   return not material.textures[2].empty();
}

constexpr bool has_env_map(const assets::msh::material& material)
{
   using assets::msh::rendertype;

   return material.rendertype == (rendertype::envmapped) or
          material.rendertype == (rendertype::normalmap_tiled_envmapped) or
          material.rendertype == (rendertype::normalmap_envmapped);
}

constexpr auto make_shader_flags(const material_pipeline_flags pipeline_flags,
                                 const assets::msh::material& material,
                                 const bool specular_visibility_in_diffuse_map,
                                 const bool static_lighting) noexcept -> shader_flags
{
   shader_flags flags = shader_flags::none;

   if (are_flags_set(pipeline_flags, material_pipeline_flags::transparent)) {
      flags |= shader_flags::transparent;
   }

   if (are_flags_set(material.flags, assets::msh::material_flags::unlit)) {
      flags |= shader_flags::unlit;
   }

   if (are_flags_set(material.flags, assets::msh::material_flags::glow)) {
      flags |= shader_flags::unlit; // Actually implementing this properly is fairly pointless without a bloom pass.
   }

   if (specular_visibility_in_diffuse_map) {
      flags |= shader_flags::specular_visibility_in_diffuse_map;
   }

   if (material.rendertype == assets::msh::rendertype::scrolling) {
      flags |= shader_flags::scrolling;
   }

   if (static_lighting) {
      flags |= shader_flags::static_lighting;
   }

   if (has_normalmap(material)) {
      flags |= shader_flags::has_normal_map;
   }

   if (has_detail_map(material)) {
      flags |= shader_flags::has_detail_map;
   }

   if (has_env_map(material)) {
      flags |= shader_flags::has_env_map;
   }

   if (material.rendertype == assets::msh::rendertype::normalmap_tiled_envmapped or
       material.rendertype == assets::msh::rendertype::normalmap_tiled) {
      flags |= shader_flags::tile_normal_map;
   }

   return flags;
}

struct alignas(16) normal_material_constants {
   shader_flags flags;
   uint32 diffuse_map;
   uint32 normal_map;
   uint32 detail_map;

   float3 specular_color;
   uint32 env_map;

   float2 scrolling_amount;
   float2 detail_scale;

   float3 env_color;
};

}

material::material(const assets::msh::material& material,
                   const bool static_lighting, gpu::device& device,
                   copy_command_list_pool& copy_command_list_pool,
                   texture_manager& texture_manager)
{
   init_textures(material, texture_manager);
   init_flags(material);
   init_resources(material, static_lighting, device, copy_command_list_pool);
}

void material::init_textures(const assets::msh::material& material,
                             texture_manager& texture_manager)
{
   texture_names.diffuse_map = lowercase_string{material.textures[0]};
   texture_names.normal_map = lowercase_string{material.textures[1]};
   texture_names.detail_map = lowercase_string{material.textures[2]};
   texture_names.env_map = lowercase_string{material.textures[3]};

   textures.diffuse_map =
      texture_manager.at_or(texture_names.diffuse_map, world_texture_dimension::_2d,
                            texture_manager.null_diffuse_map());

   if (has_normalmap(material)) {
      textures.normal_map =
         texture_manager.at_or(texture_names.normal_map, world_texture_dimension::_2d,
                               texture_manager.null_normal_map());
   }
   else {
      texture_names.normal_map.clear();
      textures.normal_map = texture_manager.null_normal_map();
   }

   if (has_detail_map(material)) {
      textures.detail_map =
         texture_manager.at_or(texture_names.detail_map, world_texture_dimension::_2d,
                               texture_manager.null_detail_map());
   }
   else {
      texture_names.detail_map.clear();
      textures.detail_map = texture_manager.null_detail_map();
   }

   if (has_env_map(material) and not texture_names.env_map.empty()) {
      textures.env_map = texture_manager.at_or(texture_names.env_map,
                                               world_texture_dimension::cube,
                                               texture_manager.null_cube_map());
   }
   else {
      texture_names.env_map.clear();
      textures.env_map = texture_manager.null_cube_map();
   }

   if (textures.diffuse_map == texture_manager.null_diffuse_map()) {
      texture_load_tokens.diffuse_map =
         texture_manager.acquire_load_token(texture_names.diffuse_map);
   }

   if (not texture_names.normal_map.empty() and
       textures.normal_map == texture_manager.null_normal_map()) {
      texture_load_tokens.normal_map =
         texture_manager.acquire_load_token(texture_names.normal_map);
   }

   if (not texture_names.detail_map.empty() and
       textures.detail_map == texture_manager.null_detail_map()) {
      texture_load_tokens.detail_map =
         texture_manager.acquire_load_token(texture_names.detail_map);
   }

   if (not texture_names.env_map.empty() and
       textures.env_map == texture_manager.null_normal_map()) {
      texture_load_tokens.env_map =
         texture_manager.acquire_load_token(texture_names.env_map);
   }
}

void material::init_flags(const assets::msh::material& material)
{
   using assets::msh::material_flags;

   if (are_flags_set(material.flags, material_flags::transparent)) {
      flags |= material_pipeline_flags::transparent;
   }

   if (are_flags_set(material.flags, material_flags::transparent_doublesided)) {
      flags |= material_pipeline_flags::transparent;
      flags |= material_pipeline_flags::doublesided;
      depth_prepass_flags |= depth_prepass_pipeline_flags::doublesided;
   }

   if (are_flags_set(material.flags, material_flags::hardedged)) {
      depth_prepass_flags |= depth_prepass_pipeline_flags::alpha_cutout;
      flags &= ~material_pipeline_flags::transparent;
   }

   if (are_flags_set(material.flags, material_flags::additive)) {
      flags |= material_pipeline_flags::additive;
   }
}

void material::init_resources(const assets::msh::material& material,
                              const bool static_lighting, gpu::device& device,
                              copy_command_list_pool& copy_command_list_pool)
{
   const bool has_specular =
      are_flags_set(material.flags, assets::msh::material_flags::specular) or
      material.rendertype == assets::msh::rendertype::specular or
      material.rendertype == assets::msh::rendertype::normalmap_specular;
   const bool is_scrolling = material.rendertype == assets::msh::rendertype::scrolling;

   const float3 specular_color =
      has_specular ? material.specular_color : float3{0.0f, 0.0f, 0.0f};
   const float2 scrolling_amount =
      is_scrolling ? float2{material.data0 / 255.0f, material.data1 / 255.0f}
                   : float2{0.f, 0.0f};
   const float2 detail_scale =
      is_scrolling ? float2{1.0f, 1.0f}
                   : float2{material.data0 > 0 ? material.data0 * 1.0f : 1.0f,
                            material.data1 > 0 ? material.data1 * 1.0f : 1.0f};

   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   constant_buffer = {device.create_buffer({.size = sizeof(normal_material_constants),
                                            .debug_name =
                                               "Material Constant Buffer"},
                                           gpu::heap_type::default_),
                      device.direct_queue};
   constant_buffer_view = device.get_gpu_virtual_address(constant_buffer.get());

   normal_material_constants constants{
      .flags = make_shader_flags(flags, material, texture_names.normal_map.empty(),
                                 static_lighting),
      .diffuse_map = textures.diffuse_map->srv_srgb.index,
      .normal_map = textures.normal_map->srv.index,
      .detail_map = textures.detail_map->srv.index,
      .specular_color = specular_color,
      .env_map = textures.env_map->srv_srgb.index,
      .scrolling_amount = scrolling_amount,
      .detail_scale = detail_scale,
      .env_color = material.specular_color};

   gpu::unique_resource_handle upload_buffer =
      {device.create_buffer({.size = sizeof(normal_material_constants),
                             .debug_name = "Material Constant Upload Buffer"},
                            gpu::heap_type::upload),
       device.background_copy_queue};

   std::byte* const upload_buffer_ptr =
      static_cast<std::byte*>(device.map(upload_buffer.get(), 0, {}));

   std::memcpy(upload_buffer_ptr, &constants, sizeof(constants));

   device.unmap(upload_buffer.get(), 0, {0, sizeof(constants)});

   command_list->copy_resource(constant_buffer.get(), upload_buffer.get());

   command_list->close();

   device.background_copy_queue.execute_command_lists(command_list.get());
}

void material::process_updated_textures(gpu::copy_command_list& command_list,
                                        const updated_textures& updated,
                                        gpu::device& device)
{
   const gpu_virtual_address constant_buffer_address =
      device.get_gpu_virtual_address(constant_buffer.get());

   if (auto new_texture = updated.find(texture_names.diffuse_map);
       new_texture != updated.end() and
       new_texture->second->dimension == world_texture_dimension::_2d) {
      textures.diffuse_map = new_texture->second;
      texture_load_tokens.diffuse_map = nullptr;

      command_list.write_buffer_immediate(constant_buffer_address +
                                             offsetof(normal_material_constants,
                                                      diffuse_map),
                                          textures.diffuse_map->srv_srgb.index);
   }

   if (auto new_texture = updated.find(texture_names.normal_map);
       new_texture != updated.end() and
       new_texture->second->dimension == world_texture_dimension::_2d) {
      textures.normal_map = new_texture->second;
      texture_load_tokens.normal_map = nullptr;

      command_list.write_buffer_immediate(constant_buffer_address +
                                             offsetof(normal_material_constants, normal_map),
                                          textures.normal_map->srv.index);
   }

   if (auto new_texture = updated.find(texture_names.detail_map);
       new_texture != updated.end() and
       new_texture->second->dimension == world_texture_dimension::_2d) {
      textures.detail_map = new_texture->second;
      texture_load_tokens.detail_map = nullptr;

      command_list.write_buffer_immediate(constant_buffer_address +
                                             offsetof(normal_material_constants, detail_map),
                                          textures.detail_map->srv.index);
   }

   if (auto new_texture = updated.find(texture_names.env_map);
       new_texture != updated.end() and
       new_texture->second->dimension == world_texture_dimension::cube) {
      textures.env_map = new_texture->second;
      texture_load_tokens.env_map = nullptr;

      command_list.write_buffer_immediate(constant_buffer_address +
                                             offsetof(normal_material_constants, env_map),
                                          textures.env_map->srv_srgb.index);
   }
}
}
