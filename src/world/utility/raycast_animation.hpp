#pragma once

#include "../world.hpp"
#include "animation.hpp"

#include <optional>

namespace we::world {

struct raycast_result_keys {
   std::optional<int32> hit;
   std::optional<int32> background_hit;
};

auto raycast_position_keys(const float3 ray_origin, const float3 ray_direction,
                           const animation& animation,
                           const animation_solver& animation_solver,
                           const float visualizer_scale) noexcept -> raycast_result_keys;

auto raycast_rotation_keys(const float3 ray_origin, const float3 ray_direction,
                           const animation& animation,
                           const animation_solver& animation_solver,
                           const float visualizer_scale) noexcept -> raycast_result_keys;

}