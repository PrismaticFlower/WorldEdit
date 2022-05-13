#pragma once

#include "types.hpp"

#include <array>
#include <string>

namespace we::world {

struct lighting_settings {
   std::array<std::string, 2> global_lights;

   float3 ambient_sky_color = {0.5f, 0.5f, 0.5f};
   float3 ambient_ground_color = {0.3f, 0.3f, 0.3f};

   std::string env_map_texture;

   bool operator==(const lighting_settings&) const noexcept = default;
};

}
