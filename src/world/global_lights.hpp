#pragma once

#include "entity_optional_link.hpp"

#include "types.hpp"

#include <array>
#include <string>

namespace we::world {

struct global_lights {
   light_optional_link global_light_1;
   light_optional_link global_light_2;

   float3 ambient_sky_color = {0.5f, 0.5f, 0.5f};
   float3 ambient_ground_color = {0.3f, 0.3f, 0.3f};

   std::string env_map_texture;

   bool operator==(const global_lights&) const noexcept = default;
};

}
