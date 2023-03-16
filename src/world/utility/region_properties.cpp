#include "region_properties.hpp"
#include "container/enum_array.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <charconv>

#include <fmt/core.h>

#include <absl/container/inlined_vector.h>

namespace we::world {

namespace {

auto parse_float(const std::string_view string) noexcept -> float
{
   float value = 0.0f;

   std::from_chars(string.data(), string.data() + string.size(), value);

   return value;
}

}

auto get_region_type(const std::string_view description) noexcept -> region_type
{
   const std::string_view type =
      string::split_first_of_exclusive(description, " ")[0];

   // clang-format off
   if (string::istarts_with(type, "deathregion")) return region_type::deathregion;
   if (string::iequals(type, "soundstream")) return region_type::soundstream;
   if (string::iequals(type, "soundstatic")) return region_type::soundstatic;
   if (string::iequals(type, "soundspace")) return region_type::soundspace;
   if (string::iequals(type, "soundtrigger")) return region_type::soundtrigger;
   if (string::iequals(type, "foleyfx")) return region_type::foleyfx;
   if (string::istarts_with(type, "shadow")) return region_type::shadow;
   if (string::iequals(type, "mapbounds")) return region_type::mapbounds;
   if (string::iequals(type, "rumble")) return region_type::rumble;
   if (string::istarts_with(type, "reflection")) return region_type::reflection;
   if (string::istarts_with(type, "rainshadow")) return region_type::rainshadow;
   if (string::istarts_with(type, "danger")) return region_type::danger;
   if (string::istarts_with(type, "DamageRegion")) return region_type::damage_region;
   if (string::istarts_with(type, "AIVis")) return region_type::ai_vis;
   if (string::iequals(type, "colorgrading")) return region_type::colorgrading;
   // clang-format on

   return region_type::typeless;
}

auto to_string(const region_type type) noexcept -> std::string
{
   // clang-format off
   switch (type) {
   default:
   case region_type::typeless: return "";
   case region_type::deathregion: return "deathregion";
   case region_type::soundstream: return "soundstream";
   case region_type::soundstatic: return "soundstatic";
   case region_type::soundspace: return "soundspace";
   case region_type::soundtrigger: return "soundtrigger";
   case region_type::foleyfx: return "foleyfx";
   case region_type::shadow: return "shadow";
   case region_type::mapbounds: return "mapbounds";
   case region_type::rumble: return "rumble";
   case region_type::reflection: return "reflection";
   case region_type::rainshadow: return "rainshadow";
   case region_type::danger: return "danger";
   case region_type::damage_region: return "DamageRegion";
   case region_type::ai_vis: return "AIVis";
   case region_type::colorgrading: return "colorgrading";
   }
   // clang-format on
}

auto get_region_allowed_shapes(const region_type type) noexcept -> region_allowed_shapes
{
   switch (type) {
   default:
      return region_allowed_shapes::all;
   case region_type::soundstream:
   case region_type::soundstatic:
      return region_allowed_shapes::sphere;
   case region_type::reflection:
      return region_allowed_shapes::box_cylinder;
   }
}

auto unpack_region_sound_stream(const std::string_view description) noexcept
   -> sound_stream_properties
{
   if (not string::istarts_with(description, "soundstream")) return {};

   const std::string_view args =
      string::split_first_of_exclusive(description, " ")[1];

   const auto [sound_name, distance_divisor_string] =
      string::split_first_of_exclusive(args, " ");

   return {.sound_name = std::string{sound_name},
           .min_distance_divisor = parse_float(distance_divisor_string)};
}

auto pack_region_sound_stream(const sound_stream_properties& properties) noexcept
   -> std::string
{
   return fmt::format("soundstream {} {:.3f}", properties.sound_name,
                      properties.min_distance_divisor);
}

auto unpack_region_sound_static(const std::string_view description) noexcept
   -> sound_static_properties
{
   if (not string::istarts_with(description, "soundstatic")) return {};

   const std::string_view args =
      string::split_first_of_exclusive(description, " ")[1];

   const auto [sound_name, distance_divisor_string] =
      string::split_first_of_exclusive(args, " ");

   return {.sound_name = std::string{sound_name},
           .min_distance_divisor = parse_float(distance_divisor_string)};
}

auto pack_region_sound_static(const sound_static_properties& properties) noexcept
   -> std::string
{
   return fmt::format("soundstatic {} {:.3f}", properties.sound_name,
                      properties.min_distance_divisor);
}

auto unpack_region_sound_space(const std::string_view description) noexcept
   -> sound_space_properties
{
   if (not string::istarts_with(description, "soundspace")) return {};

   const std::string_view sound_space_name =
      string::split_first_of_exclusive(description, " ")[1];

   return {.sound_space_name = std::string{sound_space_name}};
}

auto pack_region_sound_space(const sound_space_properties& properties) noexcept
   -> std::string
{
   return fmt::format("soundspace {}", properties.sound_space_name);
}

auto unpack_region_sound_trigger(const std::string_view description) noexcept
   -> sound_trigger_properties
{
   if (not string::istarts_with(description, "soundtrigger")) return {};

   const std::string_view region_name =
      string::split_first_of_exclusive(description, " ")[1];

   return {.region_name = std::string{region_name}};
}

auto pack_region_sound_trigger(const sound_trigger_properties& properties) noexcept
   -> std::string
{
   return fmt::format("soundtrigger {}", properties.region_name);
}

auto unpack_region_foley_fx(const std::string_view description) noexcept
   -> foley_fx_region_properties
{
   if (not string::istarts_with(description, "foleyfx")) return {};

   const std::string_view group_id =
      string::split_first_of_exclusive(description, " ")[1];

   return {.group_id = std::string{group_id}};
}

auto pack_region_foley_fx(const foley_fx_region_properties& properties) noexcept
   -> std::string
{
   return fmt::format("foleyfx {}", properties.group_id);
}

auto unpack_region_shadow(const std::string_view description) noexcept -> shadow_region_properties
{
   if (not string::istarts_with(description, "shadow")) return {};

   shadow_region_properties properties{
      .suffix = std::string{string::split_first_of_exclusive(
         string::split_first_of_exclusive(description, "shadow")[1], " ")[0]}};

   std::string_view work_string =
      string::split_first_of_exclusive(description, " ")[1];
   work_string = string::trim_leading_whitespace(work_string);

   while (not work_string.empty()) {
      auto [key_value, rest] = string::split_first_of_exclusive(work_string, " ");
      auto [key, value] = string::split_first_of_exclusive(key_value, "=");

      if (string::iequals(key, "directional")) {
         properties.directional0 = parse_float(value);
      }
      else if (string::iequals(key, "directional1")) {
         properties.directional1 = parse_float(value);
      }
      else if (string::iequals(key, "colortop")) {
         auto [r, gb] = string::split_first_of_exclusive(value, ",");
         auto [g, b] = string::split_first_of_exclusive(gb, ",");

         properties.color_top = {parse_float(r) / 255.0f, parse_float(g) / 255.0f,
                                 parse_float(b) / 255.0f};
      }
      else if (string::iequals(key, "colorbottom")) {
         auto [r, gb] = string::split_first_of_exclusive(value, ",");
         auto [g, b] = string::split_first_of_exclusive(gb, ",");

         properties.color_bottom = {parse_float(r) / 255.0f, parse_float(g) / 255.0f,
                                    parse_float(b) / 255.0f};
      }
      else if (string::iequals(key, "envmap")) {
         properties.env_map = value;
      }

      work_string = rest;
      work_string = string::trim_leading_whitespace(work_string);
   }

   return properties;
}

auto pack_region_shadow(const shadow_region_properties& properties) noexcept -> std::string
{
   absl::InlinedVector<char, 128> buffer;

   fmt::format_to(std::back_inserter(buffer), "shadow{}", properties.suffix);

   if (properties.directional0) {
      fmt::format_to(std::back_inserter(buffer), " directional={:.3f}",
                     *properties.directional0);
   }

   if (properties.directional1) {
      fmt::format_to(std::back_inserter(buffer), " directional1={:.3f}",
                     *properties.directional1);
   }

   if (properties.color_top) {
      const std::array color_top{
         static_cast<uint8>(properties.color_top->x * 255.0f + 0.5f),
         static_cast<uint8>(properties.color_top->y * 255.0f + 0.5f),
         static_cast<uint8>(properties.color_top->z * 255.0f + 0.5f),
      };

      fmt::format_to(std::back_inserter(buffer), " colortop={},{},{}",
                     color_top[0], color_top[1], color_top[2]);
   }

   if (properties.color_bottom) {
      const std::array color_bottom{
         static_cast<uint8>(properties.color_bottom->x * 255.0f + 0.5f),
         static_cast<uint8>(properties.color_bottom->y * 255.0f + 0.5f),
         static_cast<uint8>(properties.color_bottom->z * 255.0f + 0.5f),
      };

      fmt::format_to(std::back_inserter(buffer), " colorbottom={},{},{}",
                     color_bottom[0], color_bottom[1], color_bottom[2]);
   }

   if (not properties.env_map.empty()) {
      fmt::format_to(std::back_inserter(buffer), " envmap={}", properties.env_map);
   }

   return {buffer.data(), buffer.size()};
}

auto unpack_region_rumble(const std::string_view description) noexcept -> rumble_region_properties
{
   if (not string::istarts_with(description, "rumble")) return {};

   const auto [rumble_class, particle_effect] =
      string::split_first_of_exclusive(string::split_first_of_exclusive(description,
                                                                        " ")[1],
                                       " ");

   return {.rumble_class = std::string{rumble_class},
           .particle_effect = std::string{particle_effect}};
}

auto pack_region_rumble(const rumble_region_properties& properties) noexcept -> std::string
{
   return fmt::format("rumble {} {}", properties.rumble_class,
                      properties.particle_effect);
}

auto unpack_region_damage(const std::string_view description) noexcept -> damage_region_properties
{
   if (not string::istarts_with(description, "DamageRegion")) return {};

   damage_region_properties properties{};

   std::string_view work_string =
      string::split_first_of_exclusive(description, " ")[1];
   work_string = string::trim_leading_whitespace(work_string);

   while (not work_string.empty()) {
      auto [key_value, rest] = string::split_first_of_exclusive(work_string, " ");
      auto [key, value] = string::split_first_of_exclusive(key_value, "=");

      if (string::iequals(key, "damagerate")) {
         properties.damage_rate = parse_float(value);
      }
      else if (string::iequals(key, "personscale")) {
         properties.person_scale = parse_float(value);
      }
      else if (string::iequals(key, "animalscale")) {
         properties.animal_scale = parse_float(value);
      }
      else if (string::iequals(key, "droidscale")) {
         properties.droid_scale = parse_float(value);
      }
      else if (string::iequals(key, "vehiclescale")) {
         properties.vehicle_scale = parse_float(value);
      }
      else if (string::iequals(key, "buildingscale")) {
         properties.building_scale = parse_float(value);
      }
      else if (string::iequals(key, "buildingdeadscale")) {
         properties.building_dead_scale = parse_float(value);
      }
      else if (string::iequals(key, "buildingunbuiltscale")) {
         properties.building_unbuilt_scale = parse_float(value);
      }

      work_string = rest;
      work_string = string::trim_leading_whitespace(work_string);
   }

   return properties;
}

auto pack_region_damage(const damage_region_properties& properties) noexcept -> std::string
{
   const std::string_view damage_region = "DamageRegion";

   absl::InlinedVector<char, 128> buffer{damage_region.begin(), damage_region.end()};

   if (properties.damage_rate) {
      fmt::format_to(std::back_inserter(buffer), " damagerate={:.3f}",
                     *properties.damage_rate);
   }

   if (properties.person_scale) {
      fmt::format_to(std::back_inserter(buffer), " personscale={:.3f}",
                     *properties.person_scale);
   }

   if (properties.animal_scale) {
      fmt::format_to(std::back_inserter(buffer), " animalscale={:.3f}",
                     *properties.animal_scale);
   }

   if (properties.droid_scale) {
      fmt::format_to(std::back_inserter(buffer), " droidscale={:.3f}",
                     *properties.droid_scale);
   }

   if (properties.vehicle_scale) {
      fmt::format_to(std::back_inserter(buffer), " vehiclescale={:.3f}",
                     *properties.vehicle_scale);
   }

   if (properties.building_scale) {
      fmt::format_to(std::back_inserter(buffer), " buildingscale={:.3f}",
                     *properties.building_scale);
   }

   if (properties.building_dead_scale) {
      fmt::format_to(std::back_inserter(buffer), " buildingdeadscale={:.3f}",
                     *properties.building_dead_scale);
   }

   if (properties.building_unbuilt_scale) {
      fmt::format_to(std::back_inserter(buffer), " buildingunbuiltscale={:.3f}",
                     *properties.building_unbuilt_scale);
   }

   return {buffer.data(), buffer.size()};
}

auto unpack_region_ai_vis(const std::string_view description) noexcept -> ai_vis_region_properties
{
   if (not string::istarts_with(description, "AIVis")) return {};

   ai_vis_region_properties properties{};

   std::string_view work_string =
      string::split_first_of_exclusive(description, " ")[1];
   work_string = string::trim_leading_whitespace(work_string);

   while (not work_string.empty()) {
      auto [key_value, rest] = string::split_first_of_exclusive(work_string, " ");
      auto [key, value] = string::split_first_of_exclusive(key_value, "=");

      if (string::iequals(key, "crouch")) {
         properties.crouch = parse_float(value);
      }
      else if (string::iequals(key, "stand")) {
         properties.stand = parse_float(value);
      }

      work_string = rest;
      work_string = string::trim_leading_whitespace(work_string);
   }

   return properties;
}

auto pack_region_ai_vis(const ai_vis_region_properties& properties) noexcept -> std::string
{
   const std::string_view ai_vis = "AIVis";

   absl::InlinedVector<char, 128> buffer{ai_vis.begin(), ai_vis.end()};

   if (properties.crouch) {
      fmt::format_to(std::back_inserter(buffer), " crouch={:.3f}", *properties.crouch);
   }

   if (properties.stand) {
      fmt::format_to(std::back_inserter(buffer), " stand={:.3f}", *properties.stand);
   }

   return {buffer.data(), buffer.size()};
}

auto unpack_region_colorgrading(const std::string_view description) noexcept
   -> colorgrading_region_properties
{
   if (not string::istarts_with(description, "colorgrading")) return {};

   colorgrading_region_properties properties{};

   std::string_view work_string =
      string::split_first_of_exclusive(description, " ")[1];
   work_string = string::trim_leading_whitespace(work_string);

   while (not work_string.empty()) {
      auto [key_value, rest] = string::split_first_of_exclusive(work_string, " ");
      auto [key, value] = string::split_first_of_exclusive(key_value, "=");

      if (string::iequals(key, "Config")) {
         properties.config = value;
      }
      else if (string::iequals(key, "FadeLength")) {
         properties.fade_length = parse_float(value);
      }

      work_string = rest;
      work_string = string::trim_leading_whitespace(work_string);
   }

   return properties;
}

auto pack_region_colorgrading(const colorgrading_region_properties& properties) noexcept
   -> std::string
{
   return fmt::format("colorgrading Config={} FadeLength={:.3f}",
                      properties.config, properties.fade_length);
}

}
