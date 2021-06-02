#pragma once

#include "terrain.hpp"

#include <cstddef>
#include <span>

namespace we::assets::terrain {

auto read_terrain(const std::span<const std::byte> bytes) -> terrain;

}
