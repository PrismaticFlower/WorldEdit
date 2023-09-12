#pragma once

#include "id.hpp"
#include "types.hpp"

#include <string>

namespace we::world {

enum class light_type : int8 {
   directional = 1,
   point = 2,
   spot = 3,

   directional_region_box = 0 | 0b100,
   directional_region_sphere = 1 | 0b100,
   directional_region_cylinder = 2 | 0b100
};
enum class texture_addressing : int8 { wrap = 0, clamp = 1 };

struct light {
   std::string name;
   int16 layer = 0;

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

   std::string texture;

   std::string region_name;
   float3 region_size = {1.0f, 1.0f, 1.0f};
   quaternion region_rotation = {1.0f, 0.0f, 0.0f, 0.0f};

   id<light> id{};

   bool operator==(const light&) const noexcept = default;
};

using light_id = id<light>;

}
