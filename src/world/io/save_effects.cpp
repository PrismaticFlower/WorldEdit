#include "save_effects.hpp"

#include "io/output_file.hpp"
#include "math/vector_funcs.hpp"

#include <cassert>
#include <optional>

#include <fmt/format.h>

using namespace std::literals;

namespace we::world {

namespace {

/// @brief Type used to provide specialized formatting for float's to match what you'd normally find in .fx files.
struct rounded_float {
   float v;
};

}

}

template<>
struct fmt::formatter<we::world::rounded_float> : fmt::formatter<std::string_view> {
   auto format(we::world::rounded_float v, format_context& ctx) const -> fmt::appender;
};

namespace we::world {

namespace {

enum class property_type {
   bool_,
   bool_tag,
   float_,
   int32,
   int32_2,
   float2,
   float3,
   color3,
   color4,
   string,
   precipitation_type,
   water_animated_textures,
   heat_shimmer_bump_map,
   sun_flare_halo_ring,
};

union unsaved_storage {
   bool bool_ = false;
   float float_;
   int32 int32;
};

enum class indention { _1, _2 };

bool should_save(const property_type type, const void* value,
                 const std::optional<unsaved_storage> unsaved)
{
   if (not value) return false;

   if (type == property_type::string) {
      return not static_cast<const std::string*>(value)->empty();
   }

   if (not unsaved) return true;

   switch (type) {
   case property_type::bool_:
      return *static_cast<const bool*>(value) != unsaved->bool_;
   case property_type::float_:
      return *static_cast<const float*>(value) != unsaved->float_;
   case property_type::int32:
      return *static_cast<const int32*>(value) != unsaved->int32;
   }

   return true;
}

void write(io::output_file& out, indention indention_level,
           std::string_view name, const property_type type, const void* value_ptr)
{
   assert(value_ptr);

   if (not value_ptr) return;

   const std::string_view indention =
      indention_level == indention::_1 ? "\t"sv : "\t\t"sv;

   switch (type) {
   case property_type::bool_: {
      out.write_ln("{}{}({});", indention, name,
                   *static_cast<const bool*>(value_ptr) ? 1 : 0);
   } break;
   case property_type::bool_tag: {
      if (*static_cast<const bool*>(value_ptr)) {
         out.write_ln("{}{}();", indention, name);
      }
   } break;
   case property_type::float_: {
      out.write_ln("{}{}({});", indention, name,
                   rounded_float{*static_cast<const float*>(value_ptr)});
   } break;
   case property_type::int32: {
      out.write_ln("{}{}({});", indention, name, *static_cast<const int32*>(value_ptr));
   } break;
   case property_type::int32_2: {
      const std::array<int32, 2> value =
         *static_cast<const std::array<int32, 2>*>(value_ptr);

      out.write_ln("{}{}({}, {});", indention, name, value[0], value[1]);
   } break;
   case property_type::float2: {
      const float2 value = *static_cast<const float2*>(value_ptr);

      out.write_ln("{}{}({}, {});", indention, name, rounded_float{value.x},
                   rounded_float{value.y});
   } break;
   case property_type::float3: {
      const float3 value = *static_cast<const float3*>(value_ptr);

      out.write_ln("{}{}({}, {}, {});", indention, name, rounded_float{value.x},
                   rounded_float{value.y}, rounded_float{value.z});
   } break;
   case property_type::color3: {
      const float3 value = *static_cast<const float3*>(value_ptr) * 255.0f;

      out.write_ln("{}{}({}, {}, {});", indention, name, static_cast<int>(value.x),
                   static_cast<int>(value.y), static_cast<int>(value.z));
   } break;
   case property_type::color4: {
      const float4 value = *static_cast<const float4*>(value_ptr) * 255.0f;

      out.write_ln("{}{}({}, {}, {}, {});", indention, name,
                   static_cast<int>(value.x), static_cast<int>(value.y),
                   static_cast<int>(value.z), static_cast<int>(value.w));
   } break;
   case property_type::string: {
      const std::string_view value = *static_cast<const std::string*>(value_ptr);

      out.write_ln("{}{}(\"{}\");", indention, name, value);
   } break;
   case property_type::precipitation_type: {
      const precipitation_type value =
         *static_cast<const precipitation_type*>(value_ptr);

      out.write_ln("{}{}(\"{}\");", indention, name,
                   value == precipitation_type::streaks ? "Streaks"sv : "Quads"sv);
   } break;
   case property_type::water_animated_textures: {
      const water::animated_textures& value =
         *static_cast<const water::animated_textures*>(value_ptr);

      out.write_ln("{}{}(\"{}\", {}, {});", indention, name, value.prefix,
                   value.count, rounded_float{value.framerate});
   } break;
   case property_type::heat_shimmer_bump_map: {
      const heat_shimmer::bump_map& value =
         *static_cast<const heat_shimmer::bump_map*>(value_ptr);

      out.write_ln("{}{}(\"{}\", {}, {});", indention, name, value.name,
                   rounded_float{value.unknown.x}, rounded_float{value.unknown.y});
   } break;
   case property_type::sun_flare_halo_ring: {
      const sun_flare::halo_ring& value =
         *static_cast<const sun_flare::halo_ring*>(value_ptr);
      const float4 color = value.color * 255.0f;

      out.write_ln("{}{}({}, {}, {}, {}, {});", indention, name,
                   rounded_float{value.size}, static_cast<int32>(color.x),
                   static_cast<int32>(color.y), static_cast<int32>(color.z),
                   static_cast<int32>(color.w));
   } break;
   default:
      std::unreachable();
   }
}

#define UNPACK_VAR(var, member)                                                \
   var.member##_per_platform, &var.member##_pc, &var.member##_ps2, &var.member##_xbox

#define UNPACK_PC_XB_VAR(var, member)                                          \
   var.member##_per_platform, &var.member##_pc,                                \
      decltype(&var.member##_pc){nullptr}, &var.member##_xbox

struct color_property_t {};

struct tag_property_t {};

struct property {
   property(std::string_view name, std::optional<bool> per_platform,
            const bool* value_pc, const bool* value_ps2, const bool* value_xbox,
            std::optional<std::type_identity_t<bool>> unsaved_value = std::nullopt)
      : type{property_type::bool_},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox},
        unsaved_value{unsaved_value.transform(
           [](const bool v) { return unsaved_storage{.bool_ = v}; })}
   {
   }

   property(std::string_view name, tag_property_t, std::optional<bool> per_platform,
            const bool* value_pc, const bool* value_ps2, const bool* value_xbox)
      : type{property_type::bool_tag},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const float* value_pc, const float* value_ps2, const float* value_xbox,
            std::optional<std::type_identity_t<float>> unsaved_value = std::nullopt)
      : type{property_type::float_},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox},
        unsaved_value{unsaved_value.transform(
           [](const float v) { return unsaved_storage{.float_ = v}; })}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const int32* value_pc, const int32* value_ps2, const int32* value_xbox,
            std::optional<std::type_identity_t<int32>> unsaved_value = std::nullopt)
      : type{property_type::int32},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox},
        unsaved_value{unsaved_value.transform(
           [](const int32 v) { return unsaved_storage{.int32 = v}; })}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const std::array<int32, 2>* value_pc, const std::array<int32, 2>* value_ps2,
            const std::array<int32, 2>* value_xbox)
      : type{property_type::int32_2},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const float2* value_pc, const float2* value_ps2, const float2* value_xbox)
      : type{property_type::float2},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const float3* value_pc, const float3* value_ps2, const float3* value_xbox)
      : type{property_type::float3},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, color_property_t,
            std::optional<bool> per_platform, const float3* value_pc,
            const float3* value_ps2, const float3* value_xbox)
      : type{property_type::color3},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, color_property_t,
            std::optional<bool> per_platform, const float4* value_pc,
            const float4* value_ps2, const float4* value_xbox)
      : type{property_type::color4},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const std::string* value_pc, const std::string* value_ps2,
            const std::string* value_xbox)
      : type{property_type::string},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const precipitation_type* value_pc, const precipitation_type* value_ps2,
            const precipitation_type* value_xbox)
      : type{property_type::precipitation_type},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const water::animated_textures* value_pc,
            const water::animated_textures* value_ps2,
            const water::animated_textures* value_xbox)
      : type{property_type::water_animated_textures},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const heat_shimmer::bump_map* value_pc,
            const heat_shimmer::bump_map* value_ps2,
            const heat_shimmer::bump_map* value_xbox)
      : type{property_type::heat_shimmer_bump_map},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   property(std::string_view name, std::optional<bool> per_platform,
            const sun_flare::halo_ring* value_pc, const sun_flare::halo_ring* value_ps2,
            const sun_flare::halo_ring* value_xbox)
      : type{property_type::sun_flare_halo_ring},
        name{name},
        per_platform{per_platform.value_or(true)},
        value_pc{value_pc},
        value_ps2{value_ps2},
        value_xbox{value_xbox}
   {
   }

   void write_shared(io::output_file& out) const
   {
      if (per_platform) return;
      if (not should_save(type, value_pc, unsaved_value)) return;

      write(out, indention::_1, name, type, value_pc);
   }

   void write_pc(io::output_file& out) const
   {
      if (not per_platform) return;
      if (not should_save(type, value_pc, unsaved_value)) return;

      write(out, indention::_2, name, type, value_pc);
   }

   void write_ps2(io::output_file& out) const
   {
      if (not per_platform) return;
      if (not should_save(type, value_ps2, unsaved_value)) return;

      write(out, indention::_2, name, type, value_ps2);
   }

   void write_xbox(io::output_file& out) const
   {
      if (not per_platform) return;
      if (not should_save(type, value_xbox, unsaved_value)) return;

      write(out, indention::_2, name, type, value_xbox);
   }

   bool has_pc_value() const noexcept
   {
      return per_platform and value_pc;
   }

   bool has_ps2_value() const noexcept
   {
      return per_platform and value_ps2;
   }

   bool has_xbox_value() const noexcept
   {
      return per_platform and value_xbox;
   }

private:
   property_type type = property_type::bool_;
   std::string_view name;
   bool per_platform = false;
   const void* value_pc = nullptr;
   const void* value_ps2 = nullptr;
   const void* value_xbox = nullptr;

   std::optional<unsaved_storage> unsaved_value;
};

bool has_pc_properties(std::span<const property> properties) noexcept
{
   for (const auto& property : properties) {
      if (property.has_pc_value()) return true;
   }

   return false;
}

bool has_ps2_properties(std::span<const property> properties) noexcept
{
   for (const auto& property : properties) {
      if (property.has_ps2_value()) return true;
   }

   return false;
}

bool has_xbox_properties(std::span<const property> properties) noexcept
{
   for (const auto& property : properties) {
      if (property.has_xbox_value()) return true;
   }

   return false;
}

void save_properties(io::output_file& out, std::span<const property> properties)
{
   for (const auto& property : properties) {
      property.write_shared(out);
   }

   if (has_ps2_properties(properties)) {
      out.write_ln("\n\tPS2()");
      out.write_ln("\t{");

      for (const auto& property : properties) {
         property.write_ps2(out);
      }

      out.write_ln("\t}");
   }

   if (has_xbox_properties(properties)) {
      out.write_ln("\n\tXBOX()");
      out.write_ln("\t{");

      for (const auto& property : properties) {
         property.write_xbox(out);
      }

      out.write_ln("\t}");
   }

   if (has_pc_properties(properties)) {
      out.write_ln("\n\tPC()");
      out.write_ln("\t{");

      for (const auto& property : properties) {
         property.write_pc(out);
      }

      out.write_ln("\t}");
   }
}

void save_color_control(io::output_file& out, const color_control& control)
{

   out.write_ln("Effect(\"ColorControl\")");
   out.write_ln("{");

   const property properties[] = {
      {"Enable"sv, UNPACK_VAR(control, enable)},
      {"GammaBrightness"sv, UNPACK_PC_XB_VAR(control, gamma_brightness), 0.5f},
      {"GammaContrast"sv, UNPACK_PC_XB_VAR(control, gamma_contrast), 0.5f},
      {"GammaColorBalance"sv, UNPACK_PC_XB_VAR(control, gamma_color_balance), 0.5f},
      {"GammaCorrection"sv, UNPACK_PC_XB_VAR(control, gamma_correction), 0.5f},
      {"GammaHue"sv, UNPACK_PC_XB_VAR(control, gamma_hue), 0.5f},
      {"WorldBrightness"sv, UNPACK_VAR(control, world_brightness), 0.5f},
      {"WorldContrast"sv, UNPACK_VAR(control, world_contrast), 0.5f},
      {"WorldSaturation"sv, UNPACK_VAR(control, world_saturation), 0.5f},
   };

   save_properties(out, properties);

   out.write_ln("}\n");
}

void save_fog_cloud(io::output_file& out, const fog_cloud& cloud)
{

   out.write_ln("Effect(\"FogCloud\")");
   out.write_ln("{");

   const property properties[] = {
      {"Enable"sv, UNPACK_VAR(cloud, enable)},
      {"Texture"sv, UNPACK_VAR(cloud, texture)},
      {"Range"sv, UNPACK_VAR(cloud, range)},
      {"Color"sv, color_property_t{}, UNPACK_VAR(cloud, color)},
      {"Velocity"sv, UNPACK_VAR(cloud, velocity)},
      {"Rotation"sv, UNPACK_VAR(cloud, rotation)},
      {"Height"sv, UNPACK_VAR(cloud, height)},
      {"ParticleSize"sv, UNPACK_VAR(cloud, particle_size)},
      {"ParticleDensity"sv, UNPACK_VAR(cloud, particle_density)},
   };

   save_properties(out, properties);

   out.write_ln("}\n");
}

void save_wind(io::output_file& out, const wind& wind)
{

   out.write_ln("Effect(\"Wind\")");
   out.write_ln("{");

   const property properties[] = {
      {"Enable"sv, UNPACK_VAR(wind, enable)},
      {"Velocity"sv, UNPACK_VAR(wind, velocity)},
      {"VelocityRange"sv, UNPACK_VAR(wind, velocity_range)},
      {"VelocityChangeRate"sv, UNPACK_VAR(wind, velocity_change_rate)},
   };

   save_properties(out, properties);

   out.write_ln("}\n");
}

void save_precipitation(io::output_file& out, const precipitation& precipitation)
{
   out.write_ln("Effect(\"Precipitation\")");
   out.write_ln("{");

   const property properties[] = {
      // clang-format off
      {"Enable"sv, UNPACK_VAR(precipitation, enable)},
      {"Type"sv, UNPACK_VAR(precipitation, type)},
      {"Texture"sv, UNPACK_VAR(precipitation, texture)},
      {"Range"sv, UNPACK_VAR(precipitation, range)},
      {"Color"sv, color_property_t{}, UNPACK_VAR(precipitation, color)},
      {"AlphaMinMax"sv, UNPACK_VAR(precipitation, alpha_min_max)},
      {"Velocity"sv, UNPACK_VAR(precipitation, velocity)},
      {"VelocityRange"sv, UNPACK_VAR(precipitation, velocity_range)},
      {"ParticleDensity"sv, UNPACK_VAR(precipitation, particle_density)},
      {"ParticleDensityRange"sv, UNPACK_VAR(precipitation, particle_density_range)},
      {"ParticleSize"sv, UNPACK_VAR(precipitation, particle_size)},
      {"StreakLength"sv, UNPACK_VAR(precipitation, streak_length)},
      {"CameraCrossVelocityScale"sv, UNPACK_VAR(precipitation, camera_cross_velocity_scale)},
      {"CameraAxialVelocityScale"sv, UNPACK_VAR(precipitation, camera_axial_velocity_scale)},
      {"GroundEffect"sv, UNPACK_VAR(precipitation, ground_effect)},
      {"GroundEffectsPerSec"sv, UNPACK_VAR(precipitation, ground_effects_per_sec)},
      {"GroundEffectSpread"sv, UNPACK_VAR(precipitation, ground_effect_spread)},
      {"RotationRange"sv, UNPACK_VAR(precipitation, rotation_range), 0.0f},
      // clang-format on
   };

   save_properties(out, properties);

   out.write_ln("}\n");
}

void save_lightning(io::output_file& out, const lightning& lightning,
                    const lightning_bolt& bolt)
{
   out.write_ln("Effect(\"Lightning\")");
   out.write_ln("{");

   const property lightning_properties[] = {
      // clang-format off
      {"Enable"sv, UNPACK_VAR(lightning, enable)},
      {"Color"sv, color_property_t{}, UNPACK_VAR(lightning, color)},
      {"SunlightFadeFactor"sv, UNPACK_VAR(lightning, sunlight_fade_factor)},
      {"SkyDomeDarkenFactor"sv, UNPACK_VAR(lightning, sky_dome_darken_factor)},
      {"BrightnessMin"sv, UNPACK_VAR(lightning, brightness_min)},
      {"FadeTime"sv, UNPACK_VAR(lightning, fade_time)},
      {"TimeBetweenFlashesMinMax"sv, UNPACK_VAR(lightning, time_between_flashes_min_max)},
      {"TimeBetweenSubFlashesMinMax"sv, UNPACK_VAR(lightning, time_between_sub_flashes_min_max)},
      {"NumSubFlashesMinMax"sv, UNPACK_VAR(lightning, num_sub_flashes_min_max)},
      {"HorizonAngleMinMax"sv, UNPACK_VAR(lightning, horizon_angle_min_max)},
      {"SoundCrack"sv, UNPACK_VAR(lightning, sound_crack)},
      {"SoundSubCrack"sv, UNPACK_VAR(lightning, sound_sub_crack)},
      // clang-format on
   };

   save_properties(out, lightning_properties);

   out.write_ln("}\n");

   bool lightning_enabled = false;

   if (not lightning.enable_per_platform) {
      lightning_enabled |= lightning.enable_pc;
      lightning_enabled |= lightning.enable_ps2;
      lightning_enabled |= lightning.enable_xbox;
   }
   else {
      lightning_enabled = lightning.enable_pc;
   }

   if (not lightning_enabled and bolt == lightning_bolt{}) return;

   out.write_ln("LightningBolt(\"skybolt\")");
   out.write_ln("{");

   const property bolt_properties[] = {
      // clang-format off
      {"Texture"sv, UNPACK_VAR(bolt, texture)},
      {"Width"sv, UNPACK_VAR(bolt, width)},
      {"FadeTime"sv, UNPACK_VAR(bolt, fade_time)},
      {"BreakDistance"sv, UNPACK_VAR(bolt, break_distance)},
      {"TextureSize"sv, UNPACK_VAR(bolt, texture_size)},
      {"SpreadFactor"sv, UNPACK_VAR(bolt, spread_factor)},
      {"MaxBranches"sv, UNPACK_VAR(bolt, max_branches)},
      {"BranchFactor"sv, UNPACK_VAR(bolt, branch_factor)},
      {"BranchSpreadFactor"sv, UNPACK_VAR(bolt, branch_spread_factor)},
      {"BranchLength"sv, UNPACK_VAR(bolt, branch_length)},
      {"InterpolationSpeed"sv, UNPACK_VAR(bolt, interpolation_speed)},
      {"NumChildren"sv, UNPACK_VAR(bolt, num_children)},
      {"ChildBreakDistance"sv, UNPACK_VAR(bolt, child_break_distance)},
      {"ChildTextureSize"sv, UNPACK_VAR(bolt, child_texture_size)},
      {"ChildWidth"sv, UNPACK_VAR(bolt, child_width)},
      {"ChildSpreadFactor"sv, UNPACK_VAR(bolt, child_spread_factor)},
      {"Color"sv, color_property_t{}, UNPACK_VAR(bolt, color)},
      {"ChildColor"sv, color_property_t{}, UNPACK_VAR(bolt, child_color)},
      // clang-format on
   };

   save_properties(out, bolt_properties);

   out.write_ln("}\n");

   (void)bolt;
}

void save_water(io::output_file& out, const water& water)
{
   bool ocean_enabled = false;

   if (not water.ocean_enable_per_platform) {
      ocean_enabled |= water.ocean_enable_pc;
      ocean_enabled |= water.ocean_enable_ps2;
      ocean_enabled |= water.ocean_enable_xbox;
   }
   else {
      ocean_enabled = true;
   }

   out.write_ln("Effect(\"Water\")");
   out.write_ln("{");

   const property properties[] = {
      // clang-format off
      {"PatchDivisions"sv, UNPACK_VAR(water, patch_divisions)},
      {"OceanEnable"sv, UNPACK_VAR(water, ocean_enable)},
      {"OscillationEnable"sv, UNPACK_VAR(water, oscillation_enable)},
      {"DisableLowRes"sv, tag_property_t{}, UNPACK_VAR(water, disable_low_res)},
      {"WindDirection"sv, UNPACK_VAR(water, wind_direction)},
      {"WindSpeed"sv, UNPACK_VAR(water, wind_speed)},
      {"WaterRingColor"sv, color_property_t{}, UNPACK_VAR(water, water_ring_color)},
      {"WaterWakeColor"sv, color_property_t{}, UNPACK_VAR(water, water_wake_color)},
      {"WaterSplashColor"sv, color_property_t{}, UNPACK_VAR(water, water_splash_color)},
      {"Tile"sv, UNPACK_VAR(water, tile)},
      {"Velocity"sv, UNPACK_VAR(water, velocity)},
      {"LODDecimation"sv, UNPACK_VAR(water, lod_decimation)},
      {"PhillipsConstant"sv, UNPACK_VAR(water, phillips_constant)},
      {"MainTexture"sv, water.main_texture_per_platform, &water.main_texture_pc, &water.main_texture_ps2, ocean_enabled ? &water.main_texture_xbox : nullptr},
      {"FoamTexture"sv, UNPACK_VAR(water, foam_texture)},
      {"FoamTile"sv, UNPACK_VAR(water, foam_tile)},
      {"RefractionColor"sv, color_property_t{}, UNPACK_PC_XB_VAR(water, refraction_color)},   
      {"ReflectionColor"sv, color_property_t{}, UNPACK_PC_XB_VAR(water, reflection_color)},
      {"UnderwaterColor"sv, color_property_t{}, UNPACK_PC_XB_VAR(water, underwater_color)},
      {"MinDiffuseColor"sv, color_property_t{}, std::nullopt, nullptr, &water.min_diffuse_color_ps2, nullptr},
      {"MaxDiffuseColor"sv, color_property_t{}, std::nullopt, nullptr, &water.max_diffuse_color_ps2, nullptr},
      {"BorderDiffuseColor"sv, color_property_t{}, std::nullopt, nullptr, &water.border_diffuse_color_ps2, nullptr},
      {"SpecularColor"sv, color_property_t{}, std::nullopt, nullptr, &water.specular_color_ps2, nullptr},
      {"SpeckleSpecularColor"sv, color_property_t{}, std::nullopt, nullptr, &water.speckle_specular_color_ps2, nullptr},
      {"SpeckleAmbientColor"sv, color_property_t{}, std::nullopt, nullptr, &water.speckle_ambient_color_ps2, nullptr},
      {"SpeckleTextures"sv, std::nullopt, nullptr, &water.speckle_textures_ps2, nullptr},
      {"SpeckleTile"sv, std::nullopt, nullptr, &water.speckle_tile_ps2, nullptr},
      {"SpeckleScrollSpeed"sv, std::nullopt, nullptr, &water.speckle_scroll_speed_ps2, nullptr},
      {"SpeckleCoordShift"sv, std::nullopt, nullptr, &water.speckle_coord_shift_ps2, nullptr},
      {"LightAzimAndElev"sv, std::nullopt, nullptr, &water.light_azim_and_elev_ps2, nullptr},
      {"FresnelMinMax"sv, UNPACK_PC_XB_VAR(water, fresnel_min_max)},
      {"FarSceneRange"sv, std::nullopt, &water.far_scene_range_pc, nullptr, nullptr, 0.0f},
      {"NormalMapTextures"sv, UNPACK_PC_XB_VAR(water, normal_map_textures)},
      {"BumpMapTextures"sv,  std::nullopt, &water.bump_map_textures_pc, nullptr, nullptr},
      {"SpecularMaskTextures"sv, std::nullopt, &water.specular_mask_textures_pc, nullptr, nullptr},
      {"SpecularMaskTile"sv, std::nullopt, &water.specular_mask_tile_pc, nullptr, nullptr},
      {"SpecularMaskScrollSpeed"sv, std::nullopt, &water.specular_mask_scroll_speed_pc, nullptr, nullptr},
      // clang-format on
   };

   save_properties(out, properties);

   out.write_ln("}\n");
}
}

void save_effects(const std::filesystem::path& path, const effects& effects)
{
   io::output_file out{path};

   save_color_control(out, effects.color_control);
   save_fog_cloud(out, effects.fog_cloud);
   save_wind(out, effects.wind);
   save_precipitation(out, effects.precipitation);
   save_lightning(out, effects.lightning, effects.lightning_bolt);
   save_water(out, effects.water);
}

}

auto fmt::formatter<we::world::rounded_float>::format(we::world::rounded_float v,
                                                      format_context& ctx) const
   -> fmt::appender
{
   fmt::memory_buffer buf;
   fmt::format_to(std::back_inserter(buf), "{:f}", v.v);

   std::string_view string{buf};

   while (not string.ends_with(".0") and not string.empty() and string.back() == '0') {
      string.remove_suffix(1);
   }

   return fmt::formatter<std::string_view>::format(string, ctx);
}