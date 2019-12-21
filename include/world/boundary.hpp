#pragma once

#include <string>

namespace sk::world {

struct boundary {
   std::string name;
   int layer = 0;

   bool operator==(const boundary&) const noexcept = default;
};

}