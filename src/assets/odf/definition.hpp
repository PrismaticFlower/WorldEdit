#pragma once

#include "properties.hpp"

#include <string>

namespace we::assets::odf {

enum class type {
   explosion_class,
   ordnance_class,
   weapon_class,
   game_object_class
};

struct definition {
   type type = type::game_object_class;

   properties header_properties;
   properties class_properties;
   properties instance_properties;
};

}
