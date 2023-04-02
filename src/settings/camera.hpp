#pragma once

#include "types.hpp"

namespace we::settings {

struct camera {
   float move_speed = 20.0f;
   float look_sensitivity = 0.003f;
   float pan_sensitivity = 0.075f;
   float sprint_power = 1.5f;
   float step_size = 10.0f;
   float fov = 1.2217305f;
   float view_width = 256.0f;
};

}
