#pragma once

#include "properties.hpp"

namespace we::assets::odf {

enum class type {
   explosion_class,
   ordnance_class,
   weapon_class,
   game_object_class
};

struct definition {
   struct header {
      std::string_view class_label;
      std::string_view geometry_name;
   };

   type type = type::game_object_class;

   header header;
   odf::properties properties;
   odf::properties instance_properties;

   std::vector<char> storage;
};

}
