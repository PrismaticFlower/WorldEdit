#pragma once

#include "../world.hpp"
#include "io/path.hpp"
#include "output_stream.hpp"

#include <span>

namespace we::world {

void save_world(const io::path& path, const world& world,
                const std::span<const terrain_cut> terrain_cuts);

}
