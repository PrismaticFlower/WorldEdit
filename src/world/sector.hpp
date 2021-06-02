#pragma once

#include "types.hpp"

#include <string>
#include <vector>

namespace we::world {

struct sector {
   std::string name;
   int layer = 0;

   float base = 0.0f;
   float height = 0.0f;

   std::vector<float2> points;
   std::vector<std::string> objects;

   bool operator==(const sector&) const noexcept = default;
};

}
