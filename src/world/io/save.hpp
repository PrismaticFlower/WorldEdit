#pragma once

#include "../world.hpp"
#include "output_stream.hpp"

#include <filesystem>
#include <span>

namespace we::world {

void save_world(const std::filesystem::path& path, const world& world,
                const std::span<const terrain_cut> terrain_cuts);

}
