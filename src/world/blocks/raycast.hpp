#pragma once

#include "../blocks.hpp"

#include <optional>

namespace we::world {

struct raycast_block_result {
   float distance = 0.0f;
   uint32 index = 0;
   uint32 surface_index = 0;
};

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const blocks_boxes& boxes) noexcept
   -> std::optional<raycast_block_result>;

}