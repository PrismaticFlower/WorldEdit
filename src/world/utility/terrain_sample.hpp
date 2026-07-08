#pragma once

#include "../terrain.hpp"

namespace we::world {

auto sample_terrain_normal(const terrain& terrain, const float3& positionWS) noexcept
   -> float3;

}
