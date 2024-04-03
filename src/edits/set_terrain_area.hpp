#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"
#include "world/terrain.hpp"

#include <memory>

namespace we::edits {

auto make_set_terrain_area(const uint32 rect_start_x, const uint32 rect_start_y,
                           container::dynamic_array_2d<int16> rect_height_map)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_terrain_area(const uint32 rect_start_x, const uint32 rect_start_y,
                           const uint32 texture_index,
                           container::dynamic_array_2d<uint8> rect_weight_map)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_terrain_area_color_map(const uint32 rect_start_x, const uint32 rect_start_y,
                                     container::dynamic_array_2d<uint32> rect_color_map)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_terrain_area_light_map(const uint32 rect_start_x, const uint32 rect_start_y,
                                     container::dynamic_array_2d<uint32> rect_light_map)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_terrain_area_water_map(const uint32 rect_start_x, const uint32 rect_start_y,
                                     container::dynamic_array_2d<bool> rect_water_map)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_terrain_area_foliage_map(
   const uint32 rect_start_x, const uint32 rect_start_y,
   container::dynamic_array_2d<world::foliage_patch> rect_foliage_map)
   -> std::unique_ptr<edit<world::edit_context>>;

}
