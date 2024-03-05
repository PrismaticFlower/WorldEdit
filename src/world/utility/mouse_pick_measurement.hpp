#pragma once

#include "../world.hpp"

#include <span>

namespace we::world {

auto mouse_pick(const float2 mouse_position, const float2 viewport_size,
                const float4x4 view_projection_matrix,
                std::span<const measurement> measurements) noexcept
   -> std::optional<measurement_id>;

}