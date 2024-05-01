#pragma once

#include "../world.hpp"

#include <optional>

namespace we::world {

auto raycast_position_keys(const float3 ray_origin, const float3 ray_direction,
                           const animation& animation, const quaternion& base_rotation,
                           const float3& base_position,
                           const float visualizer_scale) noexcept
   -> std::optional<int32>;

auto raycast_rotation_keys(const float3 ray_origin, const float3 ray_direction,
                           const animation& animation, const quaternion& base_rotation,
                           const float3& base_position,
                           const float visualizer_scale) noexcept
   -> std::optional<int32>;

}