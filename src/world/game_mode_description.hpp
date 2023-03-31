#pragma once

#include "id.hpp"
#include "requirement_list.hpp"

#include <string>
#include <vector>

namespace we::world {

struct game_mode_description {
   std::string name;

   std::vector<int> layers;

   std::vector<requirement_list> requirements;

   bool operator==(const game_mode_description&) const noexcept = default;
};

}
