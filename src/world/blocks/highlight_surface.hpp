#pragma once

#include "../blocks.hpp"
#include "../tool_visualizers.hpp"

namespace we::world {

void highlight_surface(const block_description_box& box, const uint32 surface_index,
                       tool_visualizers& visualizers) noexcept;

}