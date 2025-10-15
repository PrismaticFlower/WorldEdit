#pragma once

#include "types.hpp"

namespace we::graphics {

struct object_class_thumbnail {
   uint32 imgui_texture_id = 0;
   float uv_left = 0.0f;
   float uv_top = 0.0f;
   float uv_right = 1.0f;
   float uv_bottom = 1.0f;
};

}