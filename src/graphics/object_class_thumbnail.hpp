#pragma once

namespace we::graphics {

struct object_class_thumbnail {
   void* imgui_texture_id = nullptr;
   float uv_left = 0.0f;
   float uv_top = 0.0f;
   float uv_right = 1.0f;
   float uv_bottom = 1.0f;
};

}