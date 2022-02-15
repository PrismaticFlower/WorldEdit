#pragma once

#include "id.hpp"

#include <string>
#include <vector>

namespace we::world {

struct gamemode_description {
   std::string name;

   std::vector<int> layers;

   id<gamemode_description> id{};

   bool operator==(const gamemode_description&) const noexcept = default;
};

using gamemode_description_id = id<gamemode_description>;

}
