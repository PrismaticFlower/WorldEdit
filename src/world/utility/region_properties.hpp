#pragma once

#include "types.hpp"

#include "../region.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace we::world {

enum class region_type {
   typeless,
   soundstream,
   soundstatic,
   soundspace,
   soundtrigger,
   foleyfx,
   shadow,
   mapbounds,
   rumble,
   reflection,
   rainshadow,
   danger,
   damage_region,
   ai_vis,
   colorgrading,
};

enum class region_allowed_shapes { all, box, sphere, box_cylinder };

struct sound_region_properties {
   std::string sound_name;
   float min_distance_divisor = 1.0f;
};

using sound_stream_properties = sound_region_properties;
using sound_static_properties = sound_region_properties;

struct sound_space_properties {
   std::string sound_space_name;
};

struct sound_trigger_properties {
   std::string region_name;
};

struct foley_fx_region_properties {
   std::string group_id;
};

struct shadow_region_properties {
   std::string suffix;

   std::optional<float> directional0;
   std::optional<float> directional1;

   std::optional<float3> color_top;
   std::optional<float3> color_bottom;

   std::string env_map;
};

struct rumble_region_properties {
   std::string rumble_class;
   std::string particle_effect;
};

struct damage_region_properties {
   std::optional<float> damage_rate;
   std::optional<float> person_scale;
   std::optional<float> animal_scale;
   std::optional<float> droid_scale;
   std::optional<float> vehicle_scale;
   std::optional<float> building_scale;
   std::optional<float> building_dead_scale;
   std::optional<float> building_unbuilt_scale;
};

struct ai_vis_region_properties {
   std::optional<float> crouch;
   std::optional<float> stand;
};

struct colorgrading_region_properties {
   std::string config;
   float fade_length = 0.0f;
};

auto get_region_type(const std::string_view description) noexcept -> region_type;

auto to_string(const region_type type) noexcept -> std::string;

auto get_region_allowed_shapes(const region_type type) noexcept -> region_allowed_shapes;

bool is_region_allowed_shape(const region_type type, const region_shape shape) noexcept;

auto unpack_region_sound_stream(const std::string_view description) noexcept
   -> sound_stream_properties;

auto pack_region_sound_stream(const sound_stream_properties& properties) noexcept
   -> std::string;

auto unpack_region_sound_static(const std::string_view description) noexcept
   -> sound_static_properties;

auto pack_region_sound_static(const sound_static_properties& properties) noexcept
   -> std::string;

auto unpack_region_sound_space(const std::string_view description) noexcept
   -> sound_space_properties;

auto pack_region_sound_space(const sound_space_properties& properties) noexcept
   -> std::string;

auto unpack_region_sound_space(const std::string_view description) noexcept
   -> sound_space_properties;

auto pack_region_sound_space(const sound_space_properties& properties) noexcept
   -> std::string;

auto unpack_region_sound_trigger(const std::string_view description) noexcept
   -> sound_trigger_properties;

auto pack_region_sound_trigger(const sound_trigger_properties& properties) noexcept
   -> std::string;

auto unpack_region_foley_fx(const std::string_view description) noexcept
   -> foley_fx_region_properties;

auto pack_region_foley_fx(const foley_fx_region_properties& properties) noexcept
   -> std::string;

auto unpack_region_shadow(const std::string_view description) noexcept
   -> shadow_region_properties;

auto pack_region_shadow(const shadow_region_properties& properties) noexcept
   -> std::string;

auto unpack_region_rumble(const std::string_view description) noexcept
   -> rumble_region_properties;

auto pack_region_rumble(const rumble_region_properties& properties) noexcept
   -> std::string;

auto unpack_region_damage(const std::string_view description) noexcept
   -> damage_region_properties;

auto pack_region_damage(const damage_region_properties& properties) noexcept
   -> std::string;

auto unpack_region_ai_vis(const std::string_view description) noexcept
   -> ai_vis_region_properties;

auto pack_region_ai_vis(const ai_vis_region_properties& properties) noexcept
   -> std::string;

auto unpack_region_colorgrading(const std::string_view description) noexcept
   -> colorgrading_region_properties;

auto pack_region_colorgrading(const colorgrading_region_properties& properties) noexcept
   -> std::string;

}