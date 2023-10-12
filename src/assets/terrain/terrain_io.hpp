#pragma once

#include "terrain.hpp"

#include <cstddef>
#include <filesystem>
#include <span>

namespace we::assets::terrain {

/// @brief Read terrain from a span of bytes.
/// @param bytes The bytes containing the terrain.
/// @return The terrain.
auto read_terrain(const std::span<const std::byte> bytes) -> terrain;

/// @brief Saves terrain to the specified path.
/// @param path The path to save the terrain to.
/// @param terrain The terrain to save.
/// @param terrain_cuts A span of terrain cuts to save into the terrain.
void save_terrain(const std::filesystem::path& path, const terrain& terrain,
                  const std::span<const terrain_cut> terrain_cuts);

}
