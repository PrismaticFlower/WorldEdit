#pragma once

#include "id.hpp"
#include "types.hpp"

#include <string>
#include <vector>

namespace we::world {

struct boundary {
   std::string name;

   bool hidden = false;

   std::vector<float3> points;

   id<boundary> id{};

   bool operator==(const boundary&) const noexcept = default;
};

using boundary_id = id<boundary>;

}
