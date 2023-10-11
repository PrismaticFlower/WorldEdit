#pragma once

#include "../terrain.hpp"
#include "types.hpp"

#include <optional>

namespace we::world {

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const terrain& terrain) noexcept -> std::optional<float>;

}
