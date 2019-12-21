#pragma once

#include <types.hpp>

#include <optional>
#include <string>

namespace sk::world {

enum class light_type { directional = 1, point = 2, spot = 3 };

struct light {
   std::string name;
   int layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   vec3 position{};

   light_type light_type = light_type::point;
   vec3 color = {1.0f, 1.0f, 1.0f};
   bool static_ = false;
   bool shadow_caster = false;
   bool specular_caster = false;

   float radius = 8.0f;
   float inner_cone_angle = 0.785398f;
   float outer_cone_angle = 0.959931f;

   vec2 directional_texture_tiling = {1.0f, 1.0f};
   vec2 directional_texture_offset = {0.0f, 0.0f};

   std::optional<std::string> texture = std::nullopt;
   std::optional<std::string> directional_region = std::nullopt;

   bool operator==(const light&) const noexcept = default;
};

}