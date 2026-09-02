#pragma once

#include "object.hpp"

#include "lowercase_string.hpp"
#include "types.hpp"

#include <string>

namespace we::world {

struct object_attached {
   float4x4 object_from_local;

   object_class_handle class_handle;

   lowercase_string class_name;
   std::string hard_point_name;
};

}
