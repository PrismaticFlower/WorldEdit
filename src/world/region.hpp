#pragma once

#include "id.hpp"
#include "types.hpp"

#include <string>

namespace we::world {

enum class region_shape { box = 0, sphere = 1, cylinder = 2 };

struct region {
   std::string name;
   int16 layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   float3 position{};
   float3 size = {1.0f, 1.0f, 1.0f};
   region_shape shape = region_shape::box;

   std::string description;

   id<region> id{};

   bool operator==(const region&) const noexcept = default;
};

using region_id = id<region>;

}
