#pragma once

#include "id.hpp"
#include "types.hpp"

#include <string>

namespace we::world {

struct measurement {
   bool hidden = false;

   float3 start{};
   float3 end{};

   std::string name;

   id<measurement> id{};

   bool operator==(const measurement&) const noexcept = default;
};

using measurement_id = id<measurement>;

}
