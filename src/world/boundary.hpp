#pragma once

#include "id.hpp"

#include <string>

namespace we::world {

struct boundary {
   std::string name;

   float2 position = {0.0f, 0.0f};
   float2 size = {256.0f, 256.0f};

   id<boundary> id{};

   bool operator==(const boundary&) const noexcept = default;
};

using boundary_id = id<boundary>;

}
