#pragma once

#include "types.hpp"

#include <span>
#include <vector>

namespace we::munge {

auto triangulate_polygon(std::span<const float3> positions, std::span<const uint32> face)
   -> std::vector<std::array<uint32, 3>>;

}