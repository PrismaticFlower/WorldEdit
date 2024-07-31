#include "material.hpp"
#include "dynamic_buffer_allocator.hpp"

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
   additive = 0b1000000000,
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

   if (are_flags_set(material.flags, assets::msh::material_flags::additive)) {
      flags |= shader_flags::transparent;
      flags |= shader_flags::additive;
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

constexpr const std::size_t material::constant_buffer_size =
   sizeof(normal_material_constants);

material::material(const assets::msh::material& material, const bool static_lighting,
                   std::span<std::byte> constant_buffer_upload_memory,
                   gpu_virtual_address constant_buffer_view,
                   texture_manager& texture_manager)
   : constant_buffer_view{constant_buffer_view}
{
   if (constant_buffer_upload_memory.size() != sizeof(normal_material_constants)) {
      std::terminate();
   }

   init_textures(material, texture_manager);
   init_flags(material);
   init_constant_buffer(material, static_lighting, constant_buffer_upload_memory);
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
       textures.env_map == texture_manager.null_cube_map()) {
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
      flags |= material_pipeline_flags::transparent;
   }

   is_transparent = are_flags_set(flags, material_pipeline_flags::transparent);
}

void material::init_constant_buffer(const assets::msh::material& material,
                                    const bool static_lighting,
                                    std::span<std::byte> constant_buffer_upload_memory)
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

   if (constant_buffer_upload_memory.size() != sizeof(normal_material_constants)) {
      std::terminate();
   }

   std::memcpy(constant_buffer_upload_memory.data(), &constants, sizeof(constants));
}

void material::process_updated_textures(gpu::copy_command_list& command_list,
                                        const updated_textures& updated)
{
   if (auto new_texture = updated.check(texture_names.diffuse_map);
       new_texture and new_texture->dimension == world_texture_dimension::_2d) {
      textures.diffuse_map = std::move(new_texture);
      texture_load_tokens.diffuse_map = nullptr;

      command_list.write_buffer_immediate(constant_buffer_view +
                                             offsetof(normal_material_constants,
                                                      diffuse_map),
                                          textures.diffuse_map->srv_srgb.index);
   }

   if (auto new_texture = updated.check(texture_names.normal_map);
       new_texture and new_texture->dimension == world_texture_dimension::_2d) {
      textures.normal_map = std::move(new_texture);
      texture_load_tokens.normal_map = nullptr;

      command_list.write_buffer_immediate(constant_buffer_view +
                                             offsetof(normal_material_constants, normal_map),
                                          textures.normal_map->srv.index);
   }

   if (auto new_texture = updated.check(texture_names.detail_map);
       new_texture and new_texture->dimension == world_texture_dimension::_2d) {
      textures.detail_map = std::move(new_texture);
      texture_load_tokens.detail_map = nullptr;

      command_list.write_buffer_immediate(constant_buffer_view +
                                             offsetof(normal_material_constants, detail_map),
                                          textures.detail_map->srv.index);
   }

   if (auto new_texture = updated.check(texture_names.env_map);
       new_texture and new_texture->dimension == world_texture_dimension::cube) {
      textures.env_map = std::move(new_texture);
      texture_load_tokens.env_map = nullptr;

      command_list.write_buffer_immediate(constant_buffer_view +
                                             offsetof(normal_material_constants, env_map),
                                          textures.env_map->srv_srgb.index);
   }
}

void material::process_updated_textures_copy(gpu::device& device,
                                             dynamic_buffer_allocator& allocator,
                                             gpu::resource_handle mesh_buffer,
                                             gpu::copy_command_list& command_list,
                                             const updated_textures& updated)
{
   const auto copy_srv_index = [&](std::size_t offset, uint32 srv_index) noexcept {
      auto allocation = allocator.allocate_and_copy(srv_index);

      command_list.copy_buffer_region(mesh_buffer,
                                      (constant_buffer_view -
                                       device.get_gpu_virtual_address(mesh_buffer)) +
                                         offset,
                                      allocation.resource, allocation.offset,
                                      sizeof(uint32));
   };

   if (auto new_texture = updated.check(texture_names.diffuse_map);
       new_texture and new_texture->dimension == world_texture_dimension::_2d) {
      textures.diffuse_map = std::move(new_texture);
      texture_load_tokens.diffuse_map = nullptr;

      copy_srv_index(offsetof(normal_material_constants, diffuse_map),
                     textures.diffuse_map->srv_srgb.index);
   }

   if (auto new_texture = updated.check(texture_names.normal_map);
       new_texture and new_texture->dimension == world_texture_dimension::_2d) {
      textures.normal_map = std::move(new_texture);
      texture_load_tokens.normal_map = nullptr;

      copy_srv_index(offsetof(normal_material_constants, normal_map),
                     textures.normal_map->srv.index);
   }

   if (auto new_texture = updated.check(texture_names.detail_map);
       new_texture and new_texture->dimension == world_texture_dimension::_2d) {
      textures.detail_map = std::move(new_texture);
      texture_load_tokens.detail_map = nullptr;

      copy_srv_index(offsetof(normal_material_constants, detail_map),
                     textures.detail_map->srv.index);
   }

   if (auto new_texture = updated.check(texture_names.env_map);
       new_texture and new_texture->dimension == world_texture_dimension::cube) {
      textures.env_map = std::move(new_texture);
      texture_load_tokens.env_map = nullptr;

      copy_srv_index(offsetof(normal_material_constants, env_map),
                     textures.env_map->srv_srgb.index);
   }
}

auto material::status(const texture_manager& texture_manager) const noexcept -> material_status
{
   bool missing_textures = false;

   if (not texture_names.diffuse_map.empty()) {
      switch (texture_manager.status(texture_names.diffuse_map,
                                     world_texture_dimension::_2d)) {
      case texture_status::ready: {
         if (texture_load_tokens.diffuse_map) {
            return material_status::ready_textures_loading;
         }
      } break;
      case texture_status::loading: {
         return material_status::ready_textures_loading;
      } break;
      case texture_status::errored:
      case texture_status::missing:
      case texture_status::dimension_mismatch: {
         missing_textures = true;
      } break;
      }
   }

   if (not texture_names.normal_map.empty()) {
      switch (texture_manager.status(texture_names.normal_map,
                                     world_texture_dimension::_2d)) {
      case texture_status::ready: {
         if (texture_load_tokens.normal_map) {
            return material_status::ready_textures_loading;
         }
      } break;
      case texture_status::loading: {
         return material_status::ready_textures_loading;
      } break;
      case texture_status::errored:
      case texture_status::missing:
      case texture_status::dimension_mismatch: {
         missing_textures = true;
      } break;
      }
   }

   if (not texture_names.detail_map.empty()) {
      switch (texture_manager.status(texture_names.detail_map,
                                     world_texture_dimension::_2d)) {
      case texture_status::ready: {
         if (texture_load_tokens.detail_map) {
            return material_status::ready_textures_loading;
         }
      } break;
      case texture_status::loading: {
         return material_status::ready_textures_loading;
      } break;
      case texture_status::errored:
      case texture_status::missing:
      case texture_status::dimension_mismatch: {
         missing_textures = true;
      } break;
      }
   }

   if (not texture_names.env_map.empty()) {
      switch (texture_manager.status(texture_names.env_map,
                                     world_texture_dimension::cube)) {
      case texture_status::ready: {
         if (texture_load_tokens.env_map) {
            return material_status::ready_textures_loading;
         }
      } break;
      case texture_status::loading: {
         return material_status::ready_textures_loading;
      } break;
      case texture_status::errored:
      case texture_status::missing:
      case texture_status::dimension_mismatch: {
         missing_textures = true;
      } break;
      }
   }

   if (missing_textures) return material_status::ready_textures_missing;

   return material_status::ready;
}

auto material::thumbnail_mesh_flags() const noexcept -> thumbnail_mesh_pipeline_flags
{
   thumbnail_mesh_pipeline_flags thumbnail_flags = thumbnail_mesh_pipeline_flags::none;

   if (are_flags_set(flags, material_pipeline_flags::doublesided)) {
      thumbnail_flags |= thumbnail_mesh_pipeline_flags::doublesided;
   }

   if (are_flags_set(flags, material_pipeline_flags::transparent)) {
      thumbnail_flags |= thumbnail_mesh_pipeline_flags::transparent;
   }

   if (are_flags_set(depth_prepass_flags, depth_prepass_pipeline_flags::alpha_cutout)) {
      thumbnail_flags |= thumbnail_mesh_pipeline_flags::alpha_cutout;
   }

   return thumbnail_flags;
}

}
