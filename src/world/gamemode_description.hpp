#pragma once

#include <string>
#include <vector>

namespace we::world {

struct gamemode_description {
   std::string name;

   std::vector<int> layers;

   bool operator==(const gamemode_description&) const noexcept = default;
};

}
