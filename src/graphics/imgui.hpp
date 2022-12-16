#pragma once

#include "gpu/rhi.hpp"

#include "imgui/imgui.h"

namespace we::graphics {

struct imgui_renderer {
   void render_draw_data(ImDrawData* draw_data, gpu::graphics_command_list& command_list);
};

}
