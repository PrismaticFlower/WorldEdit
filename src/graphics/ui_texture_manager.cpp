#include "ui_texture_manager.hpp"
#include "utility/string_icompare.hpp"

#include <absl/container/flat_hash_map.h>
#include <cassert>
#include <utility>
#include <vector>

namespace we::graphics {

struct ui_texture_manager::impl {
   void new_frame() noexcept
   {
      std::swap(_front_textures, _back_textures);
      std::swap(_front_load_tokens, _beck_load_tokens);
      std::swap(_front_garbage, _back_garbage);

      _front_textures.clear();
      _front_load_tokens.clear();
      _front_garbage.clear();
   }

   auto request(const std::string_view name, const fallback_imgui_texture fallback,
                texture_manager& texture_manager) noexcept -> uint32
   {
      if (auto it = _front_textures.find(name); it != _front_textures.end()) {
         return it->second->srv_srgb.index;
      }

      if (auto it = _back_textures.find(name); it != _back_textures.end()) {
         _front_textures.insert(*it);

         return it->second->srv_srgb.index;
      }
      const lowercase_string lowercase_name{name};

      if (auto texture = texture_manager.at_or(lowercase_name,
                                               world_texture_dimension::_2d, nullptr);
          texture) {
         _front_textures.emplace(name, texture);

         return texture->srv_srgb.index;
      }
      else {
         if (auto it = _front_load_tokens.find(name); it != _front_load_tokens.end()) {
            return fallback_texture(fallback, texture_manager);
         }

         if (auto it = _beck_load_tokens.find(name); it != _beck_load_tokens.end()) {
            _front_load_tokens.insert(*it);

            return fallback_texture(fallback, texture_manager);
         }

         _front_load_tokens.emplace(name, texture_manager.acquire_load_token(
                                             lowercase_name));
      }

      return fallback_texture(fallback, texture_manager);
   }

   void process_updated_textures(const updated_textures& updated) noexcept
   {
      updated.eval_all(
         [&](const lowercase_string& updated_name,
             const std::shared_ptr<const world_texture>& updated_texture) noexcept {
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
         });
   }

private:
   auto fallback_texture(const fallback_imgui_texture fallback,
                         texture_manager& texture_manager) const noexcept -> uint32
   {
      switch (fallback) {
      case fallback_imgui_texture::missing_diffuse:
         return texture_manager.null_diffuse_map()->srv_srgb.index;
      default:
         return texture_manager.null_color_map()->srv_srgb.index;
      }
   }

   absl::flat_hash_map<std::string, std::shared_ptr<const world_texture>> _front_textures;
   absl::flat_hash_map<std::string, std::shared_ptr<const world_texture>> _back_textures;
   absl::flat_hash_map<std::string, std::shared_ptr<const world_texture_load_token>> _front_load_tokens;
   absl::flat_hash_map<std::string, std::shared_ptr<const world_texture_load_token>> _beck_load_tokens;
   std::vector<std::shared_ptr<const world_texture>> _front_garbage;
   std::vector<std::shared_ptr<const world_texture>> _back_garbage;
};

ui_texture_manager::ui_texture_manager() noexcept = default;

ui_texture_manager::~ui_texture_manager() = default;

void ui_texture_manager::new_frame() noexcept
{
   return _impl->new_frame();
}

auto ui_texture_manager::request(const std::string_view name,
                                 const fallback_imgui_texture fallback,
                                 texture_manager& texture_manager) noexcept -> uint32
{
   return _impl->request(name, fallback, texture_manager);
}

void ui_texture_manager::process_updated_textures(const updated_textures& updated) noexcept
{
   return _impl->process_updated_textures(updated);
}

}