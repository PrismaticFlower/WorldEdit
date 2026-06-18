#pragma once

#include "types.hpp"

#include "entity_optional_link.hpp"
#include "id.hpp"

#include <string>

namespace we::world {

struct portal {
   std::string name;

   bool hidden = false;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   float3 position{};

   float width = 8.0f;
   float height = 8.0f;

   sector_optional_link sector1;
   sector_optional_link sector2;

   id<portal> id{};

   bool operator==(const portal&) const noexcept = default;
};

using portal_id = id<portal>;

}
