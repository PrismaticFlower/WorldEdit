#pragma once

#include "id.hpp"

#include <string>

namespace we::world {

struct boundary {
   std::string name;

   bool hidden = false;

   float3 position = {0.0f, 0.0f, 0.0f};
   float2 size = {8.0f, 8.0f};

   id<boundary> id{};

   bool operator==(const boundary&) const noexcept = default;
};

using boundary_id = id<boundary>;

}
