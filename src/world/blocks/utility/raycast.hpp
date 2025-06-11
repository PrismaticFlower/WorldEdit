#pragma once

#include "../../active_elements.hpp"
#include "../../blocks.hpp"

#include "utility/function_ptr.hpp"

#include <optional>

namespace we::world {

struct raycast_block_result {
   float distance = 0.0f;
   block_id id = block_id::none;
   uint32 index = 0;
   uint32 surface_index = 0;
};

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers, const blocks& blocks,
             function_ptr<bool(const block_id id) noexcept> filter = nullptr) noexcept
   -> std::optional<raycast_block_result>;
}