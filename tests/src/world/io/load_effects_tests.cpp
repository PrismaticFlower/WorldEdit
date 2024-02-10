#include "pch.h"

#include "math/vector_funcs.hpp"
#include "world/io/load_effects.hpp"

#include <bit>
#include <concepts>
#include <type_traits>

using namespace std::literals;

namespace we::world::tests {

TEST_CASE("world load effects color control", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx_color_control = R"(
Effect("ColorControl")
{
	Enable(1);
	WorldBrightness(0.46);
	WorldContrast(0.48);
	WorldSaturation(0.65);

	PC()
	{
		GammaBrightness(0.6);
		GammaContrast(0.7);
		GammaHue(0.0);
		GammaColorBalance(0.55);
	}

	PS2()
	{
		WorldBrightness(0.55);
		WorldContrast(0.40);
		WorldSaturation(0.75);
	}

	XBOX()
	{
		GammaBrightness(0.48);
		GammaContrast(0.54);
		GammaCorrection(0.52);
	}
})"sv;

   color_control loaded = load_effects(world_fx_color_control, output).color_control;

   CHECK(not loaded.enable_per_platform);

   CHECK(loaded.enable_pc);

   CHECK(loaded.gamma_brightness_per_platform);

   CHECK(loaded.gamma_brightness_pc == 0.6f);

   CHECK(loaded.gamma_brightness_xbox == 0.48f);

   CHECK(loaded.gamma_color_balance_per_platform);

   CHECK(loaded.gamma_color_balance_pc == 0.55f);

   CHECK(loaded.gamma_color_balance_xbox == 0.5f);

   CHECK(loaded.gamma_contrast_per_platform);

   CHECK(loaded.gamma_contrast_pc == 0.7f);

   CHECK(loaded.gamma_contrast_xbox == 0.54f);

   CHECK(loaded.gamma_correction_per_platform);

   CHECK(loaded.gamma_correction_pc == 0.5f);

   CHECK(loaded.gamma_hue_per_platform);

   CHECK(loaded.gamma_hue_pc == 0.0f);

   CHECK(loaded.gamma_hue_xbox == 0.5f);

   CHECK(loaded.world_brightness_per_platform);

   CHECK(loaded.world_brightness_pc == 0.46f);

   CHECK(loaded.world_brightness_ps2 == 0.55f);

   CHECK(loaded.world_brightness_xbox == 0.46f);

   CHECK(loaded.world_contrast_per_platform);

   CHECK(loaded.world_contrast_pc == 0.48f);

   CHECK(loaded.world_contrast_ps2 == 0.40f);

   CHECK(loaded.world_contrast_xbox == 0.48f);

   CHECK(loaded.world_saturation_per_platform);

   CHECK(loaded.world_saturation_pc == 0.65f);

   CHECK(loaded.world_saturation_ps2 == 0.75f);

   CHECK(loaded.world_saturation_xbox == 0.65f);
}

TEST_CASE("world load effects fog cloud", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("FogCloud")
{
	Enable(1);
	Texture("cloud");
	Range(35.0, 110.0);
	Color(168, 172, 180, 128);
	Velocity(5.0, 0.0);
	Rotation(0.1);
	Height(24.0);
	ParticleSize(32.0);
	ParticleDensity(100.0);
})"sv;

   fog_cloud loaded = load_effects(world_fx, output).fog_cloud;

   CHECK(not loaded.enable_per_platform);

   CHECK(loaded.enable_pc);

   CHECK(not loaded.texture_per_platform);

   CHECK(loaded.texture_pc == "cloud");

   CHECK(not loaded.range_per_platform);

   CHECK(loaded.range_pc == float2{35.0f, 110.0f});

   CHECK(not loaded.color_per_platform);

   CHECK(loaded.color_pc == float4{168.0f, 172.0f, 180.0f, 128.0f} / 255.0f);

   CHECK(not loaded.velocity_per_platform);

   CHECK(loaded.velocity_pc == float2{5.0f, 0.0f});

   CHECK(not loaded.rotation_per_platform);

   CHECK(loaded.rotation_pc == 0.1f);

   CHECK(not loaded.height_per_platform);

   CHECK(loaded.height_pc == 24.0f);

   CHECK(not loaded.particle_size_per_platform);

   CHECK(loaded.particle_size_pc == 32.0f);

   CHECK(not loaded.particle_density_per_platform);

   CHECK(loaded.particle_density_pc == 100.0f);
}

TEST_CASE("world load effects wind", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("Wind")
{
	Enable(1);
	Velocity(3.0, 0.6);
	VelocityRange(0.75);
	VelocityChangeRate(0.2);
})"sv;

   wind loaded = load_effects(world_fx, output).wind;

   CHECK(not loaded.enable_per_platform);

   CHECK(loaded.enable_pc);

   CHECK(not loaded.velocity_per_platform);

   CHECK(loaded.velocity_pc == float2{3.0f, 0.6f});

   CHECK(not loaded.velocity_range_per_platform);

   CHECK(loaded.velocity_range_pc == 0.75f);

   CHECK(not loaded.velocity_change_rate_per_platform);

   CHECK(loaded.velocity_change_rate_pc == 0.2f);
}

TEST_CASE("world load effects precipitation", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("Precipitation")
{
	Enable(1);
	Type("Streaks");
	Range(16.0);
	Color(200, 200, 228);
	VelocityRange(0.6);
	ParticleDensityRange(0.1);
	CameraCrossVelocityScale(0.3);
	CameraAxialVelocityScale(0.9);

	GroundEffect("huge_splash");
	GroundEffectSpread(16);

	ParticleDensity(80.0);
	Velocity(4.0);
	StreakLength(1.7);
	GroundEffectsPerSec(7);

	PS2()
	{
		AlphaMinMax(0.8, 1.0);
		ParticleSize(0.06);
	}
	XBOX()
	{
		AlphaMinMax(0.2, 0.3);
		ParticleSize(0.03);
	}
	PC()
	{
		AlphaMinMax(0.3, 0.45);
		ParticleSize(0.02);
	}
})"sv;

   precipitation loaded = load_effects(world_fx, output).precipitation;

   CHECK(not loaded.enable_per_platform);

   CHECK(loaded.enable_pc);

   CHECK(not loaded.type_per_platform);

   CHECK(loaded.type_pc == precipitation_type::streaks);

   CHECK(not loaded.range_per_platform);

   CHECK(loaded.range_pc == 16.0f);

   CHECK(not loaded.color_per_platform);

   CHECK(loaded.color_pc == float3{200.0f, 200.0f, 228.0f} / 255.0f);

   CHECK(not loaded.velocity_range_per_platform);

   CHECK(loaded.velocity_range_pc == 0.6f);

   CHECK(not loaded.particle_density_range_per_platform);

   CHECK(loaded.particle_density_range_pc == 0.1f);

   CHECK(not loaded.camera_cross_velocity_scale_per_platform);

   CHECK(loaded.camera_cross_velocity_scale_pc == 0.3f);

   CHECK(not loaded.camera_axial_velocity_scale_per_platform);

   CHECK(loaded.camera_axial_velocity_scale_pc == 0.9f);

   CHECK(not loaded.ground_effect_per_platform);

   CHECK(loaded.ground_effect_pc == "huge_splash");

   CHECK(not loaded.ground_effect_spread_per_platform);

   CHECK(loaded.ground_effect_spread_pc == 16);

   CHECK(not loaded.particle_density_per_platform);

   CHECK(loaded.particle_density_pc == 80.0f);

   CHECK(not loaded.velocity_per_platform);

   CHECK(loaded.velocity_pc == 4.0f);

   CHECK(not loaded.streak_length_per_platform);

   CHECK(loaded.streak_length_pc == 1.7f);

   CHECK(not loaded.ground_effects_per_sec_per_platform);

   CHECK(loaded.ground_effects_per_sec_pc == 7);

   CHECK(loaded.alpha_min_max_per_platform);

   CHECK(loaded.alpha_min_max_pc == float2{0.3f, 0.45f});
   CHECK(loaded.alpha_min_max_ps2 == float2{0.8f, 1.0f});
   CHECK(loaded.alpha_min_max_xbox == float2{0.2f, 0.3f});

   CHECK(loaded.particle_size_per_platform);

   CHECK(loaded.particle_size_pc == 0.02f);
   CHECK(loaded.particle_size_ps2 == 0.06f);
   CHECK(loaded.particle_size_xbox == 0.03f);
}

TEST_CASE("world load effects precipitation quads", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("Precipitation")
{
	Type("Quads");
	Texture("fx_ember");
	RotationRange(25);
})"sv;

   precipitation loaded = load_effects(world_fx, output).precipitation;

   CHECK(not loaded.type_per_platform);

   CHECK(loaded.type_pc == precipitation_type::quads);

   CHECK(not loaded.texture_per_platform);

   CHECK(loaded.texture_pc == "fx_ember");

   CHECK(not loaded.rotation_range_per_platform);

   CHECK(loaded.rotation_range_pc == 25.0f);
}

}
