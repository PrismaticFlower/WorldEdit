#include "ui_texture_manager.hpp"
#include "utility/string_icompare.hpp"

#include <cassert>
#include <utility>

namespace we::graphics {

void ui_texture_manager::new_frame() noexcept
{
   std::swap(_front_textures, _back_textures);
   std::swap(_front_garbage, _back_garbage);

   _front_textures.clear();
   _front_garbage.clear();
}

auto ui_texture_manager::request(const std::string_view name,
                                 const fallback_imgui_texture fallback,
                                 texture_manager& texture_manager) noexcept -> void*
{
   if (auto it = _back_textures.find(name); it != _back_textures.end()) {
      _front_textures.insert(*it);

      return reinterpret_cast<void*>(uint64{it->second->srv_srgb.index});
   }

   if (auto it = _front_textures.find(name); it != _front_textures.end()) {
      return reinterpret_cast<void*>(uint64{it->second->srv_srgb.index});
   }

   if (auto texture = texture_manager.at_or(lowercase_string{name},
                                            world_texture_dimension::_2d, nullptr);
       texture) {
      _front_textures.emplace(name, texture);

      return reinterpret_cast<void*>(uint64{texture->srv_srgb.index});
   }

   switch (fallback) {
   case fallback_imgui_texture::missing_diffuse:
      return reinterpret_cast<void*>(
         uint64{texture_manager.null_diffuse_map()->srv_srgb.index});
   }

   return reinterpret_cast<void*>(
      uint64{texture_manager.null_color_map()->srv_srgb.index});
}

void ui_texture_manager::process_updated_textures(const updated_textures& updated) noexcept
{
   for (const auto& [updated_name, updated_texture] : updated) {
      for (auto& [name, texture] : _front_textures) {
         if (not string::iequals(name, updated_name)) continue;

         _front_garbage.emplace_back(texture);

         texture = updated_texture;
      }

      for (auto& [name, texture] : _back_textures) {
         if (not string::iequals(name, updated_name)) continue;

         _front_garbage.emplace_back(texture);

         texture = updated_texture;
      }
   }
}

}