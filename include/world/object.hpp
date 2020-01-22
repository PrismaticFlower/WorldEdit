#pragma once

#include <types.hpp>

#include <string>
#include <vector>

namespace sk::world {

struct object {
   struct instance_property {
      std::string key;
      std::string value;

      bool operator==(const instance_property&) const noexcept = default;
   };

   std::string name;
   int layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   vec3 position{};

   int team = 0;

   std::string class_name;
   std::vector<instance_property> instance_properties;

   bool operator==(const object&) const noexcept = default;
};

}