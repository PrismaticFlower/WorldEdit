#pragma once

#include "types.hpp"

#include "utility/implementation_storage.hpp"

#include <span>
#include <string>

namespace we::assets::odf {

struct definition;

}

namespace we::world {

enum class light_class_type { point, spot };

struct light_class_flags {
   bool static_ : 1 = false;
   bool shadow_caster : 1 = false;
   bool specular_caster : 1 = false;
   bool bidirectional : 1 = false;
   bool synchronize : 1 = false;
};

struct light_class_light_description {
   light_class_type type = light_class_type::point;

   float3 color;
   float3 fixed_color;

   float range = 0.0f;
   float cos_half_outer_cone_angle = 0.0f;
   float cos_half_inner_cone_angle = 0.0f;
   float tan_half_outer_cone_angle = 0.0f;

   light_class_flags flags;

   std::string_view texture;
   std::string_view fixed_texture;

   static auto positionWS(const float4x4& world_from_object) noexcept -> float3;

   static auto directionWS(const float4x4& world_from_object) noexcept -> float3;
};

struct light_class {
   explicit light_class(const assets::odf::definition& definition) noexcept;

   light_class(const light_class&) = delete;
   light_class(light_class&&) = delete;

   ~light_class();

   void update(double delta_time) noexcept;

   auto light_description() const noexcept -> const light_class_light_description&;

   auto textures() const noexcept -> std::span<const std::string>;

   auto world_icon_size() const noexcept -> float;

private:
   struct impl;

   implementation_storage<impl, 272> _impl;
};

}