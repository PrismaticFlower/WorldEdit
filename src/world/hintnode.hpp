#pragma once

#include "types.hpp"

#include <string>

namespace sk::world {

struct hintnode {
   std::string name;
   int layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   float3 position{};

   // TODO: Hintnodes

   bool operator==(const hintnode&) const noexcept = default;
};

}
