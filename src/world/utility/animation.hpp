#pragma once

#include "../animation.hpp"

namespace we::world {

auto evaluate_animation(const animation& animation, const quaternion& base_rotation,
                        const float3& base_position, float t) noexcept -> float4x4;

auto make_position_key_for_time(const animation& animation, float t) noexcept
   -> position_key;

auto make_rotation_key_for_time(const animation& animation, float t) noexcept
   -> rotation_key;
}
