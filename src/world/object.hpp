#pragma once

#include "id.hpp"
#include "lowercase_string.hpp"
#include "object_instance_property.hpp"
#include "types.hpp"

#include <string>
#include <vector>

namespace we::world {

struct object {
   std::string name;
   int16 layer = 0;
   bool hidden = false;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   float3 position{};

   int team = 0;

   lowercase_string class_name;
   std::vector<instance_property> instance_properties;

   id<object> id{};

   bool operator==(const object&) const noexcept = default;
};

using object_id = id<object>;

}
