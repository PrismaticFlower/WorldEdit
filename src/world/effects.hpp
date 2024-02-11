#pragma once

#include "types.hpp"

#include <array>
#include <string>
#include <vector>

namespace we::world {

// Properties in the world .fx file commonly vary based on platform. But not always. These macros assist in defining
// members in our structs that can optionally vary per-platform. If these really bother you then you can easily run
// this file through the compiler with /P set and get a version without the macros.

// The macros are undefined at the end of this scope and do not escape the file.

#define PLATFORMED_VAR(type, name, ...)                                        \
   bool name##_per_platform = false;                                           \
   type name##_pc = __VA_ARGS__;                                               \
   type name##_ps2 = __VA_ARGS__;                                              \
   type name##_xbox = __VA_ARGS__;

#define PLATFORMED_PC_XB_VAR(type, name, ...)                                  \
   bool name##_per_platform = false;                                           \
   type name##_pc = __VA_ARGS__;                                               \
   type name##_xbox = __VA_ARGS__;

enum class precipitation_type { streaks, quads };

struct color_control {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_PC_XB_VAR(float, gamma_brightness, 0.5f);

   PLATFORMED_PC_XB_VAR(float, gamma_color_balance, 0.5f);

   PLATFORMED_PC_XB_VAR(float, gamma_contrast, 0.5f);

   PLATFORMED_PC_XB_VAR(float, gamma_correction, 0.5f);

   PLATFORMED_PC_XB_VAR(float, gamma_hue, 0.5f);

   PLATFORMED_VAR(float, world_brightness, 0.5f);

   PLATFORMED_VAR(float, world_contrast, 0.5f);

   PLATFORMED_VAR(float, world_saturation, 0.5f);
};

struct fog_cloud {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(std::string, texture, "");

   PLATFORMED_VAR(float2, range, {0.0f, 0.0f});

   PLATFORMED_VAR(float4, color, {1.0f, 1.0f, 1.0f, 0.5f});

   PLATFORMED_VAR(float2, velocity, {0.0f, 0.0f});

   PLATFORMED_VAR(float, rotation, 0.0f);

   PLATFORMED_VAR(float, height, 0.0f);

   PLATFORMED_VAR(float, particle_size, 28.0f);

   PLATFORMED_VAR(float, particle_density, 60.0f);
};

struct wind {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(float2, velocity, {4.0f, 0.3f});

   PLATFORMED_VAR(float, velocity_range, 1.0f);

   PLATFORMED_VAR(float, velocity_change_rate, 0.2f);
};

struct precipitation {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(precipitation_type, type, precipitation_type::streaks);

   PLATFORMED_VAR(std::string, texture, "");

   PLATFORMED_VAR(float, range, 12.5);

   PLATFORMED_VAR(float3, color, {0.8471f, 0.86275f, 0.8945f});

   PLATFORMED_VAR(float, velocity, 8.0f);

   PLATFORMED_VAR(float, velocity_range, 1.0f);

   PLATFORMED_VAR(float, particle_density, 50.0f);

   PLATFORMED_VAR(float, particle_density_range, 0.0f);

   PLATFORMED_VAR(float, particle_size, 0.02f);

   PLATFORMED_VAR(float, streak_length, 1.5f);

   PLATFORMED_VAR(float, camera_cross_velocity_scale, 0.2f);

   PLATFORMED_VAR(float, camera_axial_velocity_scale, 1.0f);

   PLATFORMED_VAR(std::string, ground_effect, "com_sfx_rainsplash");

   PLATFORMED_VAR(int32, ground_effects_per_sec, 8);

   PLATFORMED_VAR(int32, ground_effect_spread, 8);

   PLATFORMED_VAR(float2, alpha_min_max, {0.3f, 0.45f});

   PLATFORMED_VAR(float, rotation_range, 0.0f);
};

struct lightning {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(float3, color, {255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f});

   PLATFORMED_VAR(float, sunlight_fade_factor, 0.1f);

   PLATFORMED_VAR(float, sky_dome_darken_factor, 0.4f);

   PLATFORMED_VAR(float, brightness_min, 1.0f);

   PLATFORMED_VAR(float, fade_time, 0.2f);

   PLATFORMED_VAR(float2, time_between_flashes_min_max, {3.0f, 5.0f});

   PLATFORMED_VAR(float2, time_between_sub_flashes_min_max, {0.01f, 0.5f});

   bool num_sub_flashes_min_max_per_platform = false;
   std::array<int32, 2> num_sub_flashes_min_max_pc = {2, 5};
   std::array<int32, 2> num_sub_flashes_min_max_ps2 = {2, 5};
   std::array<int32, 2> num_sub_flashes_min_max_xbox = {2, 5};

   bool horizon_angle_min_max_per_platform = false;
   std::array<int32, 2> horizon_angle_min_max_pc = {30, 60};
   std::array<int32, 2> horizon_angle_min_max_ps2 = {30, 60};
   std::array<int32, 2> horizon_angle_min_max_xbox = {30, 60};

   PLATFORMED_VAR(std::string, sound_crack, "kam_amb_thunder");

   PLATFORMED_VAR(std::string, sound_sub_crack, "kam_amb_thundersub");
};

struct lightning_bolt {
   PLATFORMED_VAR(std::string, texture, "lightning");

   PLATFORMED_VAR(float, width, 30.0f);

   PLATFORMED_VAR(float, fade_time, 0.5f);

   PLATFORMED_VAR(float, break_distance, 20.0f);

   PLATFORMED_VAR(float, texture_size, 30.0f);

   PLATFORMED_VAR(float, spread_factor, 20.0f);

   PLATFORMED_VAR(float, max_branches, 2.0f);

   PLATFORMED_VAR(float, branch_factor, 0.5f);

   PLATFORMED_VAR(float, branch_spread_factor, 8.0f);

   PLATFORMED_VAR(float, branch_length, 80.0f);

   PLATFORMED_VAR(float, interpolation_speed, 0.4f);

   PLATFORMED_VAR(int32, num_children, 1);

   PLATFORMED_VAR(float, child_break_distance, 15.0f);

   PLATFORMED_VAR(float, child_texture_size, 8.0f);

   PLATFORMED_VAR(float, child_width, 1.0f);

   PLATFORMED_VAR(float, child_spread_factor, 10.0f);

   PLATFORMED_VAR(float4, color,
                  {255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f});

   PLATFORMED_VAR(float4, child_color,
                  {255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 150.0f / 255.0f});
};

struct water {
   struct animated_textures {
      std::string prefix = "";
      int32 count = 0;
      float framerate = 0.0f;

      bool operator==(const animated_textures&) const noexcept = default;
   };

   PLATFORMED_VAR(bool, ocean_enable, false);

   PLATFORMED_VAR(bool, oscillation_enable, false);

   PLATFORMED_VAR(bool, disable_low_res, false);

   bool patch_divisions_per_platform = false;
   std::array<int32, 2> patch_divisions_pc = {4, 4};
   std::array<int32, 2> patch_divisions_ps2 = {4, 4};
   std::array<int32, 2> patch_divisions_xbox = {4, 4};

   PLATFORMED_VAR(float4, water_ring_color,
                  float4{148.0f / 255.0f, 170.0f / 255.0f, 192.0f / 255.0f,
                         255.0f / 255.0f});

   PLATFORMED_VAR(float4, water_splash_color,
                  float4{192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f,
                         255.0f / 255.0f});

   PLATFORMED_VAR(float4, water_wake_color,
                  float4{192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f,
                         255.0f / 255.0f});

   PLATFORMED_VAR(int32, lod_decimation, 1);

   PLATFORMED_VAR(float2, tile, {2.0f, 2.0f});

   PLATFORMED_VAR(float2, velocity, {0.01f, 0.01f});

   PLATFORMED_VAR(std::string, main_texture, "");

   PLATFORMED_VAR(std::string, foam_texture, "");

   PLATFORMED_VAR(float2, foam_tile, {5.0f, 5.0f});

   PLATFORMED_VAR(float2, wind_direction, {0.2f, 1.0f});

   PLATFORMED_VAR(float, wind_speed, 25.0f);

   PLATFORMED_VAR(float, phillips_constant, 0.00001f);

   PLATFORMED_PC_XB_VAR(float4, refraction_color,
                        float4{5.0f / 255.0f, 217.0f / 255.0f, 255.0f / 255.0f,
                               255.0f / 255.0f});

   PLATFORMED_PC_XB_VAR(float4, reflection_color,
                        float4{57.0f / 255.0f, 90.0f / 255.0f, 138.0f / 255.0f,
                               255.0f / 255.0f});

   PLATFORMED_PC_XB_VAR(float4, underwater_color,
                        float4{61.0f / 255.0f, 124.0f / 255.0f, 144.0f / 255.0f,
                               128.0f / 255.0f});

   PLATFORMED_PC_XB_VAR(float2, fresnel_min_max, float2{0.3f, 0.6f});

   PLATFORMED_PC_XB_VAR(animated_textures, normal_map_textures,
                        {.prefix = "water_normalmap_", .count = 16, .framerate = 8.0f});

   float far_scene_range_pc = 0.0f; // Save only if not zero

   animated_textures bump_map_textures_pc = {.prefix = "water_bumpmap_",
                                             .count = 16,
                                             .framerate = 8.0f};

   animated_textures specular_mask_textures_pc = {.prefix =
                                                     "water_specularmask_",
                                                  .count = 25,
                                                  .framerate = 4.0f};

   float2 specular_mask_tile_pc = {2.0f, 2.0f};
   float2 specular_mask_scroll_speed_pc = {0.0f, 0.0f};

   float4 min_diffuse_color_ps2 =
      float4{30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 255.0f / 255.0f};
   float4 max_diffuse_color_ps2 =
      float4{70.0f / 255.0f, 70.0f / 255.0f, 70.0f / 255.0f, 255.0f / 255.0f};
   float4 border_diffuse_color_ps2 =
      float4{160.0f, 160.0f / 255.0f, 160.0f / 255.0f, 255.0f / 255.0f};
   float4 specular_color_ps2 =
      float4{60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 152.0f / 255.0f};
   float4 speckle_specular_color_ps2 =
      float4{80.0f / 255.0f, 80.0f / 255.0f, 80.0f / 255.0f, 150.0f / 255.0f};
   float4 speckle_ambient_color_ps2 =
      float4{50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 80.0f / 255.0f};

   animated_textures speckle_textures_ps2 = {.prefix = "water_specularmask_",
                                             .count = 25,
                                             .framerate = 4.0f};

   float2 speckle_tile_ps2 = {8.0f, 8.0f};
   float2 speckle_scroll_speed_ps2 = {0.0f, 0.0f};
   float2 speckle_coord_shift_ps2 = {5.0f, 5.0f};
   float2 light_azim_and_elev_ps2 = {1.0f, 0.0f};
};

struct godray {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(int32, max_godrays_in_world, 1000);

   PLATFORMED_VAR(int32, max_godrays_on_screen, 10);

   PLATFORMED_VAR(float, max_view_distance, 80.0f);

   PLATFORMED_VAR(float, fade_view_distance, 70.0f);

   PLATFORMED_VAR(float, max_length, 40.0f);

   PLATFORMED_VAR(float, offset_angle, 0.0f);

   PLATFORMED_VAR(int32, min_rays_per_godray, 3);

   PLATFORMED_VAR(int32, max_rays_per_godray, 3);

   PLATFORMED_VAR(float, radius_for_max_rays, 2.0f);

   PLATFORMED_VAR(float3, dust_velocity, {0.0f, 0.0f, 0.0f});

   PLATFORMED_VAR(std::string, texture, "fx_godray");

   PLATFORMED_VAR(float2, texture_scale, {1.0f, 1.0f});

   PLATFORMED_VAR(float3, texture_velocity, {1.0f, -0.1f, 1.0f});

   PLATFORMED_VAR(float, texture_jitter_speed, 0.05f);
};

struct heat_shimmer {
   struct bump_map {
      std::string name;
      float2 unknown = {1.0f, 1.0f}; // Presumably tiling but I can't even tell if this effect works on PC so.
   };

   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(float, world_height, 15.0f);

   PLATFORMED_VAR(float, geometry_height, 8.0f);

   PLATFORMED_VAR(float, scroll_speed, 0.08f);

   PLATFORMED_VAR(float, distortion_scale, 0.002f);

   PLATFORMED_PC_XB_VAR(int32, tessellation, 2);

   PLATFORMED_PC_XB_VAR(bump_map, bump_map, {"shimmer_waves", {1.0f, 1.0f}});

   bool per_platform_distortion_scale = true;
   float pc_distortion_scale = 0.002f;
   float ps2_distortion_scale = 0.03f;
   float xbox_distortion_scale = 2.0f;

   std::array<int32, 2> ps2_tessellation = {10, 20};
};

struct space_dust {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(std::string, texture, "spacedust1");

   PLATFORMED_VAR(float, spawn_distance, 150.0f);

   PLATFORMED_VAR(float, max_random_side_offset, 70.0f);

   PLATFORMED_VAR(float, center_dead_zone_radius, 20.0f);

   PLATFORMED_VAR(float, min_particle_scale, 0.1f);

   PLATFORMED_VAR(float, max_particle_scale, 0.5f);

   PLATFORMED_VAR(float, spawn_delay, 0.02f);

   PLATFORMED_VAR(float, reference_speed, 0.02f);

   PLATFORMED_VAR(float, dust_particle_speed, 100.0f);

   PLATFORMED_VAR(float, speed_particle_min_length, 2.0f);

   PLATFORMED_VAR(float, speed_particle_max_length, 12.0f);

   PLATFORMED_VAR(float, particle_length_min_speed, 35.0f);

   PLATFORMED_VAR(float, particle_length_max_speed, 170.0f);
};

struct world_shadow_map {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(std::string, texture, "");

   PLATFORMED_VAR(std::string, light_name, "");

   PLATFORMED_VAR(float, texture_scale, 70.0f);

   PLATFORMED_VAR(float, animation_frequency, 0.01f);

   PLATFORMED_VAR(float2, animation_amplitude0, {2.0f, 0.0f});

   PLATFORMED_VAR(float2, animation_amplitude1, {0.05f, -0.1f});
};

struct blur {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(float, constant_blend, 0.25f);

   PLATFORMED_VAR(float, down_size_factor, 0.25f);

   PLATFORMED_VAR(float2, min_max_depth, {0.0f, 1.0f}); // Save if != {0, 1}

   PLATFORMED_VAR(int, mode, 0); // Save if != 0
};

struct motion_blur {
   PLATFORMED_VAR(bool, enable, true);
};

struct scope_blur {
   PLATFORMED_VAR(bool, enable, true);
};

struct hdr {
   PLATFORMED_VAR(bool, enable, true);

   PLATFORMED_VAR(float, down_size_factor, 0.25f);

   PLATFORMED_VAR(int32, num_bloom_passes, 5);

   PLATFORMED_VAR(float, max_total_weight, 1.2f);

   PLATFORMED_VAR(float, glow_threshold, 0.5f);

   PLATFORMED_VAR(float, glow_factor, 1.0f);
};

struct shadow {
   PLATFORMED_VAR(bool, enable, false);

   PLATFORMED_VAR(bool, blur_enable, false);

   PLATFORMED_VAR(float, intensity, 0.1f);
};

struct sun_flare {
   struct halo_ring {
      float size = 0.0f;
      float4 color = {255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f};
   };

   PLATFORMED_VAR(float2, angle, {110.0f, -10.0f});

   PLATFORMED_VAR(float3, color, {255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f});

   PLATFORMED_VAR(float, size, 5.0f);

   PLATFORMED_VAR(float, flare_out_size, 20.0f);

   PLATFORMED_VAR(int32, num_flare_outs, 40);

   PLATFORMED_VAR(int32, initial_flare_out_alpha, 70);

   PLATFORMED_VAR(halo_ring, halo_inner_ring,
                  {0.0f, {255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f}});

   PLATFORMED_VAR(halo_ring, halo_middle_ring,
                  {10.0f,
                   {246.0f / 255.0f, 237.0f / 255.0f, 144.0f / 255.0f, 128.0f / 255.0f}});

   PLATFORMED_VAR(halo_ring, halo_outter_ring,
                  {30.0f, {130.0f / 255.0f, 76.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f}});

   PLATFORMED_VAR(float3, spike_color,
                  {230.0f / 255.0f, 230.0f / 0.0f, 255.0f / 128.0f});

   PLATFORMED_VAR(float, spike_size, 20.0f);
};

struct effects {
   color_control color_control;

   fog_cloud fog_cloud;

   wind wind;

   precipitation precipitation;

   lightning lightning;

   lightning_bolt lightning_bolt;

   water water;

   godray godray;

   heat_shimmer heat_shimmer;

   space_dust space_dust;

   world_shadow_map world_shadow_map;

   blur blur;

   motion_blur motion_blur;

   scope_blur scope_blur;

   hdr hdr;

   shadow shadow;

   std::vector<sun_flare> sun_flares;
};

#undef PLATFORMED_VAR
#undef PLATFORMED_PC_XB_VAR

}