#pragma once

#include <string>

namespace we::world {

struct boundary {
   std::string name;

   bool operator==(const boundary&) const noexcept = default;
};

}
