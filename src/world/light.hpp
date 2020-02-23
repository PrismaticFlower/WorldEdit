#pragma once

#include "types.hpp"

#include <optional>
#include <string>

namespace sk::world {

enum class light_type : int8 { directional = 1, point = 2, spot = 3 };
enum class texture_addressing : int8 { wrap = 0, clamp = 1 };

struct light {
   std::string name;
   int layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   float3 position{};

   float3 color = {1.0f, 1.0f, 1.0f};
   bool static_ = false;
   bool shadow_caster = false;
   bool specular_caster = false;
   light_type light_type = light_type::point;
   texture_addressing texture_addressing = texture_addressing::clamp;

   float range = 8.0f;
   float inner_cone_angle = 0.785398f;
   float outer_cone_angle = 0.959931f;

   float2 directional_texture_tiling = {1.0f, 1.0f};
   float2 directional_texture_offset = {0.0f, 0.0f};

   std::optional<std::string> texture = std::nullopt;
   std::optional<std::string> directional_region = std::nullopt;

   bool operator==(const light&) const noexcept = default;
};

}