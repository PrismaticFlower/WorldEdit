#pragma once

#include "../blocks.hpp"

#include <optional>

namespace we::world {

auto find_block(const blocks_boxes& boxes, const block_box_id id)
   -> std::optional<uint32>;

}
