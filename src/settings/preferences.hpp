#pragma once

#include <string>

namespace we::settings {

auto detect_default_text_editor() -> std::string;

struct preferences {
   static std::string default_text_editor;

   float cursor_placement_reenable_distance = 10.0f;
   float terrain_height_brush_stickiness = 1.0f;
   std::string text_editor = default_text_editor;
   std::string game_install_path;

   bool dont_save_world_gamemodes = false;
   bool dont_save_world_effects = false;
   bool save_world_boundary_bf1_format = false;
   bool save_blocks_into_layer = true;
   bool dont_ask_to_add_animation_to_group = false;
   bool dont_extrapolate_new_animation_keys = false;

   bool disable_double_click_select = false;

   bool operator==(const preferences&) const noexcept = default;
};

}