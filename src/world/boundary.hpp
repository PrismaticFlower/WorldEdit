#pragma once

#include "id.hpp"

#include <string>

namespace we::world {

struct boundary {
   std::string name;

   id<boundary> id{};

   bool operator==(const boundary&) const noexcept = default;
};

using boundary_id = id<boundary>;

}
