#pragma once

#include "../world.hpp"

#include "io/path.hpp"

namespace we::world {

void export_height_map(const io::path& path, const world& world);

void export_texture_weight_map(const io::path& path,
                               const container::dynamic_array_2d<uint8>& map);

void export_color_map(const io::path& path,
                      const container::dynamic_array_2d<uint32>& map);

}
