#pragma once

#include "../../blocks.hpp"
#include "../../tool_visualizers.hpp"

namespace we::world {

void highlight_surface(const blocks& blocks, const block_type type,
                       const uint32 block_index, const uint32 surface_index,
                       tool_visualizers& visualizers) noexcept;

}