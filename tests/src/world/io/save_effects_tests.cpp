#include "pch.h"

#include "io/read_file.hpp"
#include "math/vector_funcs.hpp"
#include "world/io/save_effects.hpp"

namespace we::world::tests {

TEST_CASE("world save effects", "[World][IO]")
{
   const std::string_view expected_fx = R"(Effect("ColorControl")
{
	Enable(1);
	WorldBrightness(0.46);
	WorldContrast(0.48);
	WorldSaturation(0.65);

	XBOX()
	{
		GammaBrightness(0.48);
		GammaContrast(0.54);
		GammaCorrection(0.52);
		GammaHue(0.52);
	}

	PC()
	{
		GammaBrightness(0.6);
		GammaContrast(0.7);
		GammaColorBalance(0.55);
	}
}

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
}

Effect("Wind")
{
	Enable(1);
	Velocity(3.0, 0.6);
	VelocityRange(0.75);
	VelocityChangeRate(0.2);
}

Effect("Precipitation")
{
	Enable(1);
	Type("Streaks");
	Range(16.0);
	Color(200, 200, 228);
	Velocity(4.0);
	VelocityRange(0.6);
	ParticleDensity(80.0);
	ParticleDensityRange(0.1);
	StreakLength(1.7);
	CameraCrossVelocityScale(0.3);
	CameraAxialVelocityScale(0.9);
	GroundEffect("huge_splash");
	GroundEffectsPerSec(7);
	GroundEffectSpread(16);

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
}

Effect("Lightning")
{
	Enable(1);
	Color(220, 220, 255);
	SunlightFadeFactor(0.2);
	SkyDomeDarkenFactor(0.3);
	BrightnessMin(0.7);
	FadeTime(0.3);
	TimeBetweenFlashesMinMax(2.0, 4.0);
	TimeBetweenSubFlashesMinMax(0.02, 0.7);
	NumSubFlashesMinMax(1, 8);
	HorizonAngleMinMax(20, 80);
	SoundCrack("test_amb_thunder");
	SoundSubCrack("test_amb_thundersub");
}

LightningBolt("skybolt")
{
	Texture("lightning2");
	Width(10.0);
	FadeTime(0.25);
	BreakDistance(30.0);
	TextureSize(40.0);
	SpreadFactor(30.0);
	MaxBranches(3.0);
	BranchFactor(0.75);
	BranchSpreadFactor(16);
	BranchLength(160.0);
	InterpolationSpeed(0.7);
	NumChildren(3);
	ChildBreakDistance(5.0);
	ChildTextureSize(4.0);
	ChildWidth(2.0);
	ChildSpreadFactor(20.0);
	Color(200, 200, 255, 255);
	ChildColor(200, 200, 255, 150);
}

Effect("Water")
{
	PatchDivisions(8, 8);
	OceanEnable(0);
	OscillationEnable(0);
	DisableLowRes();
	WindDirection(0.2, 1.0);
	WindSpeed(25.0);
	WaterRingColor(148, 170, 200, 255);
	WaterWakeColor(200, 200, 200, 255);
	WaterSplashColor(200, 200, 200, 255);
	PhillipsConstant(0.00001);
	FoamTile(5.0, 5.0);

	PS2()
	{
		Tile(1.0, 1.0);
		Velocity(0.0, 0.0);
		LODDecimation(8);
		MainTexture("water_ps2");
		MinDiffuseColor(45, 45, 45, 255);
		MaxDiffuseColor(85, 85, 85, 255);
		BorderDiffuseColor(25, 25, 25, 255);
		SpecularColor(80, 80, 80, 152);
		SpeckleSpecularColor(100, 100, 100, 150);
		SpeckleAmbientColor(75, 75, 75, 80);
		SpeckleTextures("water_ps2_specularmask_", 32, 2.0);
		SpeckleTile(4.0, 4.0);
		SpeckleScrollSpeed(0.1, 0.1);
		SpeckleCoordShift(2.0, 2.0);
		LightAzimAndElev(0.5, 0.0);
	}

	XBOX()
	{
		Tile(4.0, 4.0);
		Velocity(0.04, 0.008);
		LODDecimation(2);
		RefractionColor(110, 135, 139, 255);
		ReflectionColor(110, 135, 139, 255);
		UnderwaterColor(96, 96, 88, 128);
		FresnelMinMax(0.3, 0.6);
		NormalMapTextures("water_xbox_normalmap_", 4, 16.0);
	}

	PC()
	{
		Tile(3.0, 3.0);
		Velocity(0.02, 0.02);
		LODDecimation(1);
		MainTexture("water_pc");
		RefractionColor(5, 230, 255, 255);
		ReflectionColor(57, 100, 138, 255);
		UnderwaterColor(61, 128, 144, 128);
		FresnelMinMax(0.1, 0.75);
		FarSceneRange(1500.0);
		NormalMapTextures("water_pc_normalmap_", 8, 4.0);
		BumpMapTextures("water_pc_bumpmap_", 32, 16.0);
		SpecularMaskTextures("water_pc_specularmask_", 16, 8.0);
		SpecularMaskTile(4.0, 4.0);
		SpecularMaskScrollSpeed(0.2, 0.2);
	}
}

)";

   effects effects{
      .color_control =
         {
            .enable_pc = true,

            .gamma_brightness_per_platform = true,
            .gamma_brightness_pc = 0.6f,
            .gamma_brightness_xbox = 0.48f,

            .gamma_color_balance_per_platform = true,
            .gamma_color_balance_pc = 0.55f,
            .gamma_color_balance_xbox = 0.5f,

            .gamma_contrast_per_platform = true,
            .gamma_contrast_pc = 0.7f,
            .gamma_contrast_xbox = 0.54f,

            .gamma_correction_per_platform = true,
            .gamma_correction_pc = 0.5f,
            .gamma_correction_xbox = 0.52f,

            .gamma_hue_per_platform = true,
            .gamma_hue_pc = 0.5f,
            .gamma_hue_xbox = 0.52f,

            .world_brightness_pc = 0.46f,
            .world_contrast_pc = 0.48f,
            .world_saturation_pc = 0.65f,
         },

      .fog_cloud =
         {
            .enable_pc = true,
            .texture_pc = "cloud",
            .range_pc = {35.0f, 110.0f},
            .color_pc = float4{168.0f, 172.0f, 180.0f, 128.0f} / 255.0f,
            .velocity_pc = {5.0f, 0.0f},
            .rotation_pc = 0.1f,
            .height_pc = 24.0f,
            .particle_size_pc = 32.0f,
            .particle_density_pc = 100.0f,
         },

      .wind =
         {
            .enable_pc = true,
            .velocity_pc = {3.0f, 0.6f},
            .velocity_range_pc = 0.75f,
            .velocity_change_rate_pc = 0.2f,
         },

      .precipitation =
         {
            .enable_pc = true,
            .type_pc = precipitation_type::streaks,
            .range_pc = 16.0f,
            .color_pc = float3{200.0f, 200.0f, 228.0f} / 255.0f,
            .velocity_pc = 4.0f,
            .velocity_range_pc = 0.6f,
            .particle_density_pc = 80.0f,
            .particle_density_range_pc = 0.1f,

            .particle_size_per_platform = true,
            .particle_size_pc = 0.02f,
            .particle_size_ps2 = 0.06f,
            .particle_size_xbox = 0.03f,

            .streak_length_pc = 1.7f,
            .camera_cross_velocity_scale_pc = 0.3f,
            .camera_axial_velocity_scale_pc = 0.9f,
            .ground_effect_pc = "huge_splash",
            .ground_effects_per_sec_pc = 7,
            .ground_effect_spread_pc = 16,

            .alpha_min_max_per_platform = true,
            .alpha_min_max_pc = {0.3f, 0.45f},
            .alpha_min_max_ps2 = {0.8f, 1.0f},
            .alpha_min_max_xbox = {0.2f, 0.3f},
         },

      .lightning =
         {
            .enable_pc = true,
            .color_pc = float3{220.0f, 220.0f, 255.0f} / 255.0f,
            .sunlight_fade_factor_pc = 0.2f,
            .sky_dome_darken_factor_pc = 0.3f,
            .brightness_min_pc = 0.7f,
            .fade_time_pc = 0.3f,
            .time_between_flashes_min_max_pc = {2.0f, 4.0f},
            .time_between_sub_flashes_min_max_pc = {0.02f, 0.7f},
            .num_sub_flashes_min_max_pc = {1, 8},
            .horizon_angle_min_max_pc = {20, 80},
            .sound_crack_pc = "test_amb_thunder",
            .sound_sub_crack_pc = "test_amb_thundersub",
         },

      .lightning_bolt =
         {
            .texture_pc = "lightning2",
            .width_pc = 10.0f,
            .fade_time_pc = 0.25f,
            .break_distance_pc = 30.0f,
            .texture_size_pc = 40.0f,
            .spread_factor_pc = 30.0f,
            .max_branches_pc = 3.0f,
            .branch_factor_pc = 0.75f,
            .branch_spread_factor_pc = 16,
            .branch_length_pc = 160.0f,
            .interpolation_speed_pc = 0.7f,
            .num_children_pc = 3,
            .child_break_distance_pc = 5.0f,
            .child_texture_size_pc = 4.0f,
            .child_width_pc = 2.0f,
            .child_spread_factor_pc = 20.0f,
            .color_pc = float4{200.0f, 200.0f, 255.0f, 255.0f} / 255.0f,
            .child_color_pc = float4{200.0f, 200.0f, 255.0f, 150.0f} / 255.0f,
         },

      .water =
         {
            .ocean_enable_pc = false,
            .oscillation_enable_pc = false,
            .disable_low_res_pc = true,
            .patch_divisions_pc = {8, 8},
            .water_ring_color_pc = float4{148.0f, 170.0f, 200.0f, 255.0f} / 255.0f,
            .water_wake_color_pc = float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f,
            .water_splash_color_pc = float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f,

            .lod_decimation_per_platform = true,
            .lod_decimation_pc = 1,
            .lod_decimation_ps2 = 8,
            .lod_decimation_xbox = 2,

            .tile_per_platform = true,
            .tile_pc = {3.0f, 3.0f},
            .tile_ps2 = {1.0f, 1.0f},
            .tile_xbox = {4.0f, 4.0f},

            .velocity_per_platform = true,
            .velocity_pc = {0.02f, 0.02f},
            .velocity_ps2 = {0.0f, 0.0f},
            .velocity_xbox = {0.04f, 0.008f},

            .main_texture_per_platform = true,
            .main_texture_pc = "water_pc",
            .main_texture_ps2 = "water_ps2",

            .refraction_color_per_platform = true,
            .refraction_color_pc = float4{5.0f, 230.0f, 255.0f, 255.0f} / 255.0f,
            .refraction_color_xbox = float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f,

            .reflection_color_per_platform = true,
            .reflection_color_pc = float4{57.0f, 100.0f, 138.0f, 255.0f} / 255.0f,
            .reflection_color_xbox = float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f,

            .underwater_color_per_platform = true,
            .underwater_color_pc = float4{61.0f, 128.0f, 144.0f, 128.0f} / 255.0f,
            .underwater_color_xbox = float4{96.0f, 96.0f, 88.0f, 128.0f} / 255.0f,

            .fresnel_min_max_per_platform = true,
            .fresnel_min_max_pc = {0.1f, 0.75f},
            .fresnel_min_max_xbox = {0.3f, 0.6f},

            .normal_map_textures_per_platform = true,
            .normal_map_textures_pc = {"water_pc_normalmap_", 8, 4.0f},
            .normal_map_textures_xbox = {"water_xbox_normalmap_", 4, 16.0f},

            .far_scene_range_pc = 1500.0f,

            .bump_map_textures_pc = {"water_pc_bumpmap_", 32, 16.0f},
            .specular_mask_textures_pc = {"water_pc_specularmask_", 16, 8.0f},
            .specular_mask_tile_pc = {4.0f, 4.0f},
            .specular_mask_scroll_speed_pc = {0.2f, 0.2f},

            .min_diffuse_color_ps2 = float4{45.0f, 45.0f, 45.0f, 255.0f} / 255.0f,
            .max_diffuse_color_ps2 = float4{85.0f, 85.0f, 85.0f, 255.0f} / 255.0f,
            .border_diffuse_color_ps2 = float4{25.0f, 25.0f, 25.0f, 255.0f} / 255.0f,
            .specular_color_ps2 = float4{80.0f, 80.0f, 80.0f, 152.0f} / 255.0f,
            .speckle_specular_color_ps2 = float4{100.0f, 100.0f, 100.0f, 150.0f} / 255.0f,
            .speckle_ambient_color_ps2 = float4{75.0f, 75.0f, 75.0f, 80.0f} / 255.0f,
            .speckle_textures_ps2 = {"water_ps2_specularmask_", 32, 2.0f},
            .speckle_tile_ps2 = {4.0f, 4.0f},
            .speckle_scroll_speed_ps2 = {0.1f, 0.1f},
            .speckle_coord_shift_ps2 = {2.0f, 2.0f},
            .light_azim_and_elev_ps2 = {0.5f, 0.0f},
         },
   };

   std::filesystem::create_directory(L"temp/world");

   save_effects(L"temp/world/test.fx", effects);

   const auto written_fx = io::read_file_to_string(L"temp/world/test.fx");

   CHECK(written_fx == expected_fx);
}

}