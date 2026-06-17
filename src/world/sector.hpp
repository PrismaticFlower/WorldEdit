#pragma once

#include "id.hpp"
#include "types.hpp"

#include <string>
#include <vector>

namespace we::world {

struct sector {
   std::string name;

   bool hidden = false;

   float base = 0.0f;
   float height = 0.0f;

   std::vector<float2> points;
   std::vector<uint32> objects;
   std::vector<std::string> objects_broken_links;

   id<sector> id{};

   bool operator==(const sector&) const noexcept = default;
};

using sector_id = id<sector>;

}
