#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"
#include "world/terrain.hpp"

#include <memory>

namespace we::edits {

auto make_set_terrain_area(const uint32 rect_start_x, const uint32 rect_start_y,
                           container::dynamic_array_2d<int16> rect_height_map)
   -> std::unique_ptr<edit<world::edit_context>>;

}
