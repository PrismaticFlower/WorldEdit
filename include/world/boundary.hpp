#pragma once

#include <string>

namespace sk::world {

struct boundary {
   std::string name;

   bool operator==(const boundary&) const noexcept = default;
};

}