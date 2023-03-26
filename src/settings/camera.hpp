#pragma once

#include "types.hpp"

namespace we::settings {

struct camera {
   float move_speed = 20.0f;
   float look_sensitivity = 0.007f;
   float pan_sensitivity = 0.075f;
   float sprint_power = 1.5f;
};

}
