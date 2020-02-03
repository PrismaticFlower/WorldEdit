#pragma once

#include "types.hpp"

#include <array>
#include <optional>
#include <string>

namespace sk::world {

struct lighting_settings {
   std::array<std::string, 2> global_lights;

   vec3 ambient_sky_color = {0.5f, 0.5f, 0.5f};
   vec3 ambient_ground_color = {0.3f, 0.3f, 0.3f};

   std::optional<std::string> env_map_texture = std::nullopt;

   bool operator==(const lighting_settings&) const noexcept = default;
};

}