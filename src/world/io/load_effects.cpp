#include "load_effects.hpp"

#include "assets/config/io.hpp"
#include "load_failure.hpp"
#include "math/vector_funcs.hpp"
#include "utility/string_icompare.hpp"

#include <span>

#include <fmt/core.h>

using namespace std::literals;

namespace we::world {

namespace {

void read_bool(const assets::config::values& values, void* pc_value,
               void* ps2_value, void* xbox_value)
{
   const bool value = values.get<int>(0) != 0;

   if (pc_value) *static_cast<bool*>(pc_value) = value;
   if (ps2_value) *static_cast<bool*>(ps2_value) = value;
   if (xbox_value) *static_cast<bool*>(xbox_value) = value;
}

void read_bool_tag([[maybe_unused]] const assets::config::values& values,
                   void* pc_value, void* ps2_value, void* xbox_value)
{
   if (pc_value) *static_cast<bool*>(pc_value) = true;
   if (ps2_value) *static_cast<bool*>(ps2_value) = true;
   if (xbox_value) *static_cast<bool*>(xbox_value) = true;
}

void read_int32(const assets::config::values& values, void* pc_value,
                void* ps2_value, void* xbox_value)
{
   const int32 value = values.get<int32>(0);

   if (pc_value) *static_cast<int32*>(pc_value) = value;
   if (ps2_value) *static_cast<int32*>(ps2_value) = value;
   if (xbox_value) *static_cast<int32*>(xbox_value) = value;
}

void read_int32_2(const assets::config::values& values, void* pc_value,
                  void* ps2_value, void* xbox_value)
{
   const std::array<int32, 2> value = {values.get<int32>(0), values.get<int32>(1)};

   if (pc_value) *static_cast<std::array<int32, 2>*>(pc_value) = value;
   if (ps2_value) *static_cast<std::array<int32, 2>*>(ps2_value) = value;
   if (xbox_value) *static_cast<std::array<int32, 2>*>(xbox_value) = value;
}

void read_float(const assets::config::values& values, void* pc_value,
                void* ps2_value, void* xbox_value)
{
   const float value = values.get<float>(0);

   if (pc_value) *static_cast<float*>(pc_value) = value;
   if (ps2_value) *static_cast<float*>(ps2_value) = value;
   if (xbox_value) *static_cast<float*>(xbox_value) = value;
}

void read_float2(const assets::config::values& values, void* pc_value,
                 void* ps2_value, void* xbox_value)
{
   const float2 value = {values.get<float>(0), values.get<float>(1)};

   if (pc_value) *static_cast<float2*>(pc_value) = value;
   if (ps2_value) *static_cast<float2*>(ps2_value) = value;
   if (xbox_value) *static_cast<float2*>(xbox_value) = value;
}

void read_color3(const assets::config::values& values, void* pc_value,
                 void* ps2_value, void* xbox_value)
{
   const float3 value =
      float3{values.get<float>(0), values.get<float>(1), values.get<float>(2)} / 255.0f;

   if (pc_value) *static_cast<float3*>(pc_value) = value;
   if (ps2_value) *static_cast<float3*>(ps2_value) = value;
   if (xbox_value) *static_cast<float3*>(xbox_value) = value;
}

void read_color4(const assets::config::values& values, void* pc_value,
                 void* ps2_value, void* xbox_value)
{
   const float4 value = float4{values.get<float>(0), values.get<float>(1),
                               values.get<float>(2), values.get<float>(3)} /
                        255.0f;

   if (pc_value) *static_cast<float4*>(pc_value) = value;
   if (ps2_value) *static_cast<float4*>(ps2_value) = value;
   if (xbox_value) *static_cast<float4*>(xbox_value) = value;
}

void read_string(const assets::config::values& values, void* pc_value,
                 void* ps2_value, void* xbox_value)
{
   const std::string_view value = values.get<std::string_view>(0);

   if (pc_value) *static_cast<std::string*>(pc_value) = value;
   if (ps2_value) *static_cast<std::string*>(ps2_value) = value;
   if (xbox_value) *static_cast<std::string*>(xbox_value) = value;
}

void read_precipitation_type(const assets::config::values& values,
                             void* pc_value, void* ps2_value, void* xbox_value)
{
   const std::string_view value = values.get<std::string_view>(0);

   precipitation_type type{};

   if (string::iequals(value, "streaks"sv)) {
      type = precipitation_type::streaks;
   }
   else if (string::iequals(value, "quads"sv)) {
      type = precipitation_type::quads;
   }
   else {
      throw load_failure{fmt::format("Invalid Precipitation Type: '{}'", value)};
   }

   if (pc_value) *static_cast<precipitation_type*>(pc_value) = type;
   if (ps2_value) *static_cast<precipitation_type*>(ps2_value) = type;
   if (xbox_value) *static_cast<precipitation_type*>(xbox_value) = type;
}

void read_animated_textures(const assets::config::values& values,
                            void* pc_value, void* ps2_value, void* xbox_value)
{
   using animated_textures = water::animated_textures;

   animated_textures texture{.prefix = values.get<std::string>(0),
                             .count = values.get<int32>(1),
                             .framerate = values.get<float>(2)};

   if (pc_value) *static_cast<animated_textures*>(pc_value) = texture;
   if (ps2_value) *static_cast<animated_textures*>(ps2_value) = texture;
   if (xbox_value) *static_cast<animated_textures*>(xbox_value) = texture;
}

struct color_property_t {};

struct tag_property_t {};

struct property {
   property(std::string_view name, bool* per_platform_value, bool* pc_value,
            bool* ps2_value, bool* xbox_value)
      : name{name},
        per_platform_value{per_platform_value},
        read_value{read_bool},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, tag_property_t, bool* per_platform_value,
            bool* pc_value, bool* ps2_value, bool* xbox_value)
      : name{name},
        per_platform_value{per_platform_value},
        read_value{read_bool_tag},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, bool* per_platform_value, int32* pc_value,
            int32* ps2_value, int32* xbox_value)
      : name{name},
        per_platform_value{per_platform_value},
        read_value{read_int32},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, bool* per_platform_value,
            std::array<int32, 2>* pc_value, std::array<int32, 2>* ps2_value,
            std::array<int32, 2>* xbox_value)
      : name{name},
        per_platform_value{per_platform_value},
        read_value{read_int32_2},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, bool* per_platform_value, float* pc_value,
            float* ps2_value, float* xbox_value)
      : name{name},
        read_value{read_float},
        per_platform_value{per_platform_value},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, bool* per_platform_value, float2* pc_value,
            float2* ps2_value, float2* xbox_value)
      : name{name},
        read_value{read_float2},
        per_platform_value{per_platform_value},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, color_property_t, bool* per_platform_value,
            float3* pc_value, float3* ps2_value, float3* xbox_value)
      : name{name},
        read_value{read_color3},
        per_platform_value{per_platform_value},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, color_property_t, bool* per_platform_value,
            float4* pc_value, float4* ps2_value, float4* xbox_value)
      : name{name},
        read_value{read_color4},
        per_platform_value{per_platform_value},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, bool* per_platform_value,
            std::string* pc_value, std::string* ps2_value, std::string* xbox_value)
      : name{name},
        read_value{read_string},
        per_platform_value{per_platform_value},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, bool* per_platform_value, precipitation_type* pc_value,
            precipitation_type* ps2_value, precipitation_type* xbox_value)
      : name{name},
        read_value{read_precipitation_type},
        per_platform_value{per_platform_value},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, bool* per_platform_value,
            water::animated_textures* pc_value, water::animated_textures* ps2_value,
            water::animated_textures* xbox_value)
      : name{name},
        read_value{read_animated_textures},
        per_platform_value{per_platform_value},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   void read(const assets::config::values& values) const
   {
      read_value(values, pc_value, ps2_value, xbox_value);
   }

   void read_pc(const assets::config::values& values) const
   {
      read_value(values, pc_value, nullptr, nullptr);

      if (per_platform_value) *per_platform_value = true;
   }

   void read_ps2(const assets::config::values& values) const
   {
      read_value(values, nullptr, ps2_value, nullptr);

      if (per_platform_value) *per_platform_value = true;
   }

   void read_xbox(const assets::config::values& values) const
   {
      read_value(values, nullptr, nullptr, xbox_value);

      if (per_platform_value) *per_platform_value = true;
   }

   std::string_view name = "";

private:
   void (*read_value)(const assets::config::values& values, void* pc, void* ps2,
                      void* xbox) = nullptr;
   bool* per_platform_value = nullptr;
   void* pc_value = nullptr;
   void* ps2_value = nullptr;
   void* xbox_value = nullptr;
};

#define UNPACK_VAR(var, member)                                                \
   &var.member##_per_platform, &var.member##_pc, &var.member##_ps2, &var.member##_xbox

#define UNPACK_PC_XB_VAR(var, member)                                          \
   &var.member##_per_platform, &var.member##_pc, nullptr, &var.member##_xbox

void read_node(assets::config::node& node, std::span<const property> properties)
{
   for (auto& key_node : node) {
      for (const property& prop : properties) {
         if (not string::iequals(key_node.key, prop.name)) continue;

         prop.read(key_node.values);

         break;
      }
   }

   for (auto& platform_node : node) {
      if (string::iequals(platform_node.key, "PC")) {
         for (auto& key_node : platform_node) {
            for (const property& prop : properties) {
               if (not string::iequals(key_node.key, prop.name)) continue;

               prop.read_pc(key_node.values);

               break;
            }
         }
      }
      else if (string::iequals(platform_node.key, "PS2")) {
         for (auto& key_node : platform_node) {
            for (const property& prop : properties) {
               if (not string::iequals(key_node.key, prop.name)) continue;

               prop.read_ps2(key_node.values);

               break;
            }
         }
      }
      else if (string::iequals(platform_node.key, "XBOX")) {
         for (auto& key_node : platform_node) {
            for (const property& prop : properties) {
               if (not string::iequals(key_node.key, prop.name)) continue;

               prop.read_xbox(key_node.values);

               break;
            }
         }
      }
   }
}

auto read_color_control(assets::config::node& node) -> color_control
{
   color_control control;

   const property properties[] = {
      {"Enable"sv, UNPACK_VAR(control, enable)},
      {"GammaBrightness"sv, UNPACK_PC_XB_VAR(control, gamma_brightness)},
      {"GammaColorBalance"sv, UNPACK_PC_XB_VAR(control, gamma_color_balance)},
      {"GammaContrast"sv, UNPACK_PC_XB_VAR(control, gamma_contrast)},
      {"GammaCorrection"sv, UNPACK_PC_XB_VAR(control, gamma_correction)},
      {"GammaHue"sv, UNPACK_PC_XB_VAR(control, gamma_hue)},
      {"WorldBrightness"sv, UNPACK_VAR(control, world_brightness)},
      {"WorldContrast"sv, UNPACK_VAR(control, world_contrast)},
      {"WorldSaturation"sv, UNPACK_VAR(control, world_saturation)},
   };

   read_node(node, properties);

   return control;
}

auto read_fog_cloud(assets::config::node& node) -> fog_cloud
{
   fog_cloud cloud;

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

   read_node(node, properties);

   return cloud;
}

auto read_wind(assets::config::node& node) -> wind
{

   wind wind;

   const property properties[] = {
      {"Enable"sv, UNPACK_VAR(wind, enable)},
      {"Velocity"sv, UNPACK_VAR(wind, velocity)},
      {"VelocityRange"sv, UNPACK_VAR(wind, velocity_range)},
      {"VelocityChangeRate"sv, UNPACK_VAR(wind, velocity_change_rate)},
   };

   read_node(node, properties);

   return wind;
}

auto read_precipitation(assets::config::node& node) -> precipitation
{

   precipitation precipitation;

   const property properties[] = {
      // clang-format off
      {"Enable"sv, UNPACK_VAR(precipitation, enable)},
      {"Type"sv, UNPACK_VAR(precipitation, type)},
      {"Texture"sv, UNPACK_VAR(precipitation, texture)},
      {"Range"sv, UNPACK_VAR(precipitation, range)},
      {"Color"sv, color_property_t{}, UNPACK_VAR(precipitation, color)},
      {"VelocityRange"sv, UNPACK_VAR(precipitation, velocity_range)},
      {"ParticleDensityRange"sv, UNPACK_VAR(precipitation, particle_density_range)},
      {"CameraCrossVelocityScale"sv, UNPACK_VAR(precipitation, camera_cross_velocity_scale)},
      {"CameraAxialVelocityScale"sv, UNPACK_VAR(precipitation, camera_axial_velocity_scale)},
      {"GroundEffect"sv, UNPACK_VAR(precipitation, ground_effect)},
      {"GroundEffectSpread"sv, UNPACK_VAR(precipitation, ground_effect_spread)},
      {"ParticleDensity"sv, UNPACK_VAR(precipitation, particle_density)},
      {"Velocity"sv, UNPACK_VAR(precipitation, velocity)},
      {"StreakLength"sv, UNPACK_VAR(precipitation, streak_length)},
      {"GroundEffectsPerSec"sv, UNPACK_VAR(precipitation, ground_effects_per_sec)},
      {"AlphaMinMax"sv, UNPACK_VAR(precipitation, alpha_min_max)},
      {"ParticleSize"sv, UNPACK_VAR(precipitation, particle_size)},
      {"RotationRange"sv, UNPACK_VAR(precipitation, rotation_range)},
      // clang-format on
   };

   read_node(node, properties);

   return precipitation;
}

auto read_lightning(assets::config::node& node) -> lightning
{

   lightning lightning;

   const property properties[] = {
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

   read_node(node, properties);

   return lightning;
}

auto read_lightning_bolt(assets::config::node& node) -> lightning_bolt
{

   lightning_bolt bolt;

   const property properties[] = {
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

   read_node(node, properties);

   return bolt;
}

auto read_water(assets::config::node& node) -> water
{
   water water;

   const property properties[] = {
      // clang-format off
      {"BorderDiffuseColor"sv, color_property_t{}, nullptr, nullptr, &water.border_diffuse_color_ps2, nullptr},
      {"BumpMapTextures"sv,  nullptr, &water.bump_map_textures_pc, nullptr, nullptr},
      {"DisableLowRes"sv, tag_property_t{}, UNPACK_VAR(water, disable_low_res)},
      {"FarSceneRange"sv, nullptr, &water.far_scene_range_pc, nullptr, nullptr},
      {"FoamTexture"sv, UNPACK_VAR(water, foam_texture)},
      {"FoamTile"sv, UNPACK_VAR(water, foam_tile)},
      {"FresnelMinMax"sv, UNPACK_PC_XB_VAR(water, fresnel_min_max)},
      {"LODDecimation"sv, UNPACK_VAR(water, lod_decimation)},
      {"LightAzimAndElev"sv, nullptr, nullptr, &water.light_azim_and_elev_ps2, nullptr},
      {"MainTexture"sv, UNPACK_VAR(water, main_texture)},
      {"MaxDiffuseColor"sv, color_property_t{}, nullptr, nullptr, &water.max_diffuse_color_ps2, nullptr},
      {"MinDiffuseColor"sv, color_property_t{}, nullptr, nullptr, &water.min_diffuse_color_ps2, nullptr},
      {"NormalMapTextures"sv, UNPACK_PC_XB_VAR(water, normal_map_textures)},
      {"OceanEnable"sv, UNPACK_VAR(water, ocean_enable)},
      {"OscillationEnable"sv, UNPACK_VAR(water, oscillation_enable)},
      {"PatchDivisions"sv, UNPACK_VAR(water, patch_divisions)},
      {"PhillipsConstant"sv, UNPACK_VAR(water, phillips_constant)},
      {"ReflectionColor"sv, color_property_t{}, UNPACK_PC_XB_VAR(water, reflection_color)},
      {"RefractionColor"sv, color_property_t{}, UNPACK_PC_XB_VAR(water, refraction_color)},
      {"SpeckleAmbientColor"sv, color_property_t{}, nullptr, nullptr, &water.speckle_ambient_color_ps2, nullptr},
      {"SpeckleCoordShift"sv, nullptr, nullptr, &water.speckle_coord_shift_ps2, nullptr},
      {"SpeckleScrollSpeed"sv, nullptr, nullptr, &water.speckle_scroll_speed_ps2, nullptr},
      {"SpeckleSpecularColor"sv, color_property_t{}, nullptr, nullptr, &water.speckle_specular_color_ps2, nullptr},
      {"SpeckleTextures"sv, nullptr, nullptr, &water.speckle_textures_ps2, nullptr},
      {"SpeckleTile"sv, nullptr, nullptr, &water.speckle_tile_ps2, nullptr},
      {"SpecularColor"sv, color_property_t{}, nullptr, nullptr, &water.specular_color_ps2, nullptr},
      {"SpecularMaskScrollSpeed"sv, nullptr, &water.specular_mask_scroll_speed_pc, nullptr, nullptr},
      {"SpecularMaskTextures"sv, nullptr, &water.specular_mask_textures_pc, nullptr, nullptr},
      {"SpecularMaskTile"sv, nullptr, &water.specular_mask_tile_pc, nullptr, nullptr},
      {"Tile"sv, UNPACK_VAR(water, tile)},
      {"UnderwaterColor"sv, color_property_t{}, UNPACK_PC_XB_VAR(water, underwater_color)},
      {"Velocity"sv, UNPACK_VAR(water, velocity)},
      {"WaterRingColor"sv, color_property_t{}, UNPACK_VAR(water, water_ring_color)},
      {"WaterSplashColor"sv, color_property_t{}, UNPACK_VAR(water, water_splash_color)},
      {"WaterWakeColor"sv, color_property_t{}, UNPACK_VAR(water, water_wake_color)},
      {"WindDirection"sv, UNPACK_VAR(water, wind_direction)},
      {"WindSpeed"sv, UNPACK_VAR(water, wind_speed)},
      // clang-format on
   };

   read_node(node, properties);

   for (std::string* texture : {
           &water.main_texture_pc,
           &water.main_texture_ps2,
           &water.main_texture_xbox,
           &water.foam_texture_pc,
           &water.foam_texture_ps2,
           &water.foam_texture_xbox,
        }) {
      if (string::iends_with(*texture, ".tga"sv)) {
         texture->resize(texture->size() - ".tga"sv.size());
      }
   }

   return water;
}

}

auto load_effects(const std::string_view str, [[maybe_unused]] output_stream& output)
   -> effects
{
   effects effects{};

   try {
      for (auto& key_node : assets::config::read_config(str)) {
         if (string::iequals(key_node.key, "Effect"sv)) {
            const std::string_view effect = key_node.values.get<std::string_view>(0);

            if (string::iequals(effect, "ColorControl"sv)) {
               effects.color_control = read_color_control(key_node);
            }
            else if (string::iequals(effect, "FogCloud"sv)) {
               effects.fog_cloud = read_fog_cloud(key_node);
            }
            else if (string::iequals(effect, "Wind"sv)) {
               effects.wind = read_wind(key_node);
            }
            else if (string::iequals(effect, "Precipitation"sv)) {
               effects.precipitation = read_precipitation(key_node);
            }
            else if (string::iequals(effect, "Lightning"sv)) {
               effects.lightning = read_lightning(key_node);
            }
            else if (string::iequals(effect, "Water"sv)) {
               effects.water = read_water(key_node);
            }
            else {
               throw load_failure{
                  fmt::format("Unknown Effect ('{}') in world effects file.", effect)};
            }
         }
         else if (string::iequals(key_node.key, "LightningBolt"sv)) {
            const std::string_view effect = key_node.values.get<std::string_view>(0);

            if (string::iequals(effect, "skybolt"sv)) {
               effects.lightning_bolt = read_lightning_bolt(key_node);
            }
         }
         else {
            throw load_failure{
               fmt::format("Unknown entry ('{}') in world effects file.",
                           key_node.key)};
         }
      }
   }
   catch (std::exception& e) {
      throw load_failure{e.what()};
   }

   return effects;
}

}