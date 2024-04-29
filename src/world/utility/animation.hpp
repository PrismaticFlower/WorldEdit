#pragma once

#include "../animation.hpp"

namespace we::world {

auto evaluate_animation(const animation& animation, const quaternion& base_rotation,
                        const float3& base_position, float t) noexcept -> float4x4;

}
