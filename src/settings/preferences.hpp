#pragma once

#include <string>

namespace we::settings {

auto detect_default_text_editor() -> std::string;

struct preferences {
   static std::string default_text_editor;

   float cursor_placement_reenable_distance = 10.0f;
   float terrain_height_brush_stickiness = 1.0f;
   std::string text_editor = default_text_editor;
};

}