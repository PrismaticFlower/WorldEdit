#pragma once

#include "fallback_imgui_texture.hpp"
#include "texture_manager.hpp"

#include <absl/container/flat_hash_map.h>
#include <vector>

namespace we::graphics {

struct ui_texture_manager {
   /// @brief Mark the start of a new frame. Releasing any textures that were not used in the previous frame.
   void new_frame() noexcept;

   /// @brief Requests a texture. A returned texture will be kept alive until at least the next frame.
   /// @param name Name of the texture to get.
   /// @param texture_manager The texture_manager.
   /// @return The texture ID to pass to ImGui.
   auto request(const std::string_view name, const fallback_imgui_texture fallback,
                texture_manager& texture_manager) noexcept -> void*;

   /// @brief Process any updated textures.
   /// @param updated
   void process_updated_textures(const updated_textures& updated) noexcept;

private:
   absl::flat_hash_map<std::string, std::shared_ptr<const world_texture>> _front_textures;
   absl::flat_hash_map<std::string, std::shared_ptr<const world_texture>> _back_textures;
   std::vector<std::shared_ptr<const world_texture>> _front_garbage;
   std::vector<std::shared_ptr<const world_texture>> _back_garbage;
};

}
