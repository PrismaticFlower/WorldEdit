#pragma once

#include "../blocks.hpp"

#include <optional>

namespace we::world {

auto find_block(const blocks& blocks, const block_id id) -> std::optional<uint32>;

}
