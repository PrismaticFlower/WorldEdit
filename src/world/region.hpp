#pragma once

#include "types.hpp"

#include <string>

namespace sk::world {

enum class region_shape { box = 0, sphere = 1, cylinder = 2 };

struct region {
   std::string name;
   int layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   float3 position{};
   float3 size{};
   region_shape shape = region_shape::box;

   std::string description;

   bool operator==(const region&) const noexcept = default;
};

}