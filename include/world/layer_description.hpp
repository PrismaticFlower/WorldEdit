#pragma once

#include <string>

namespace sk::world {

struct layer_description {
   std::string name;

   bool operator==(const layer_description&) const noexcept = default;
};

}