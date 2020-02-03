#pragma once

#include "types.hpp"

#include <optional>
#include <string>

namespace sk::world {

struct portal {
   std::string name;
   int layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   vec3 position{};

   float width = 0.0f;
   float height = 0.0f;

   std::optional<std::string> sector1;
   std::optional<std::string> sector2;

   bool operator==(const portal&) const noexcept = default;
};

}