#pragma once

#include "camera.hpp"
#include "graphics.hpp"
#include "preferences.hpp"
#include "ui.hpp"

namespace we::settings {

struct settings {
   graphics graphics;
   camera camera;
   ui ui;
   preferences preferences;
};

void show_imgui_editor(settings& settings, bool& open, float display_scale) noexcept;

}
