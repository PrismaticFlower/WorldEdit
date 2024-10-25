#pragma once

#include "types.hpp"

#include "io/path.hpp"

#include <span>

namespace we::assets::texture {

struct env_map_view {
   uint32 length = 0;
   uint32 row_pitch = 0;
   uint32 item_pitch = 0;

   std::span<const std::byte> data;
};

/// @brief Save an env map from a texture array in a 6x3 layout in a .tga file. Can throw we::io::error on failure.
/// @param path The path to the file to save the env map to.
/// @param env_map The env map to save.
void save_env_map(const io::path& path, const env_map_view env_map);

}