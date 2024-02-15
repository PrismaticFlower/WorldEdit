#include "pch.h"

#include "math/vector_funcs.hpp"
#include "world/io/load_effects.hpp"

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
   CHECK(loaded.gamma_correction_xbox == 0.52f);

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

TEST_CASE("world load effects lightning", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
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
})"sv;

   lightning loaded = load_effects(world_fx, output).lightning;

   CHECK(not loaded.enable_per_platform);
   CHECK(loaded.enable_pc);

   CHECK(not loaded.color_per_platform);
   CHECK(loaded.color_pc == float3{220.0f, 220.0f, 255.0f} / 255.0f);

   CHECK(not loaded.sunlight_fade_factor_per_platform);
   CHECK(loaded.sunlight_fade_factor_pc == 0.2f);

   CHECK(not loaded.sky_dome_darken_factor_per_platform);
   CHECK(loaded.sky_dome_darken_factor_pc == 0.3f);

   CHECK(not loaded.brightness_min_per_platform);
   CHECK(loaded.brightness_min_pc == 0.7f);

   CHECK(not loaded.fade_time_per_platform);
   CHECK(loaded.fade_time_pc == 0.3f);

   CHECK(not loaded.time_between_flashes_min_max_per_platform);
   CHECK(loaded.time_between_flashes_min_max_pc == float2{2.0f, 4.0f});

   CHECK(not loaded.time_between_sub_flashes_min_max_per_platform);
   CHECK(loaded.time_between_sub_flashes_min_max_pc == float2{0.02f, 0.7f});

   CHECK(not loaded.num_sub_flashes_min_max_per_platform);
   CHECK(loaded.num_sub_flashes_min_max_pc == std::array{1, 8});

   CHECK(not loaded.horizon_angle_min_max_per_platform);
   CHECK(loaded.horizon_angle_min_max_pc == std::array{20, 80});

   CHECK(not loaded.sound_crack_per_platform);
   CHECK(loaded.sound_crack_pc == "test_amb_thunder");

   CHECK(not loaded.sound_sub_crack_per_platform);
   CHECK(loaded.sound_sub_crack_pc == "test_amb_thundersub");
}

TEST_CASE("world load effects lightning bolt", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
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
})"sv;

   lightning_bolt loaded = load_effects(world_fx, output).lightning_bolt;

   CHECK(not loaded.texture_per_platform);
   CHECK(loaded.texture_pc == "lightning2");

   CHECK(not loaded.width_per_platform);
   CHECK(loaded.width_pc == 10.0f);

   CHECK(not loaded.fade_time_per_platform);
   CHECK(loaded.fade_time_pc == 0.25f);

   CHECK(not loaded.break_distance_per_platform);
   CHECK(loaded.break_distance_pc == 30.0f);

   CHECK(not loaded.texture_size_per_platform);
   CHECK(loaded.texture_size_pc == 40.0f);

   CHECK(not loaded.spread_factor_per_platform);
   CHECK(loaded.spread_factor_pc == 30.0f);

   CHECK(not loaded.max_branches_per_platform);
   CHECK(loaded.max_branches_pc == 3.0f);

   CHECK(not loaded.branch_factor_per_platform);
   CHECK(loaded.branch_factor_pc == 0.75f);

   CHECK(not loaded.branch_spread_factor_per_platform);
   CHECK(loaded.branch_spread_factor_pc == 16.0f);

   CHECK(not loaded.branch_length_per_platform);
   CHECK(loaded.branch_length_pc == 160.0f);

   CHECK(not loaded.interpolation_speed_per_platform);
   CHECK(loaded.interpolation_speed_pc == 0.7f);

   CHECK(not loaded.num_children_per_platform);
   CHECK(loaded.num_children_pc == 3);

   CHECK(not loaded.child_break_distance_per_platform);
   CHECK(loaded.child_break_distance_pc == 5.0f);

   CHECK(not loaded.child_texture_size_per_platform);
   CHECK(loaded.child_texture_size_pc == 4.0f);

   CHECK(not loaded.child_width_per_platform);
   CHECK(loaded.child_width_pc == 2.0f);

   CHECK(not loaded.child_spread_factor_per_platform);
   CHECK(loaded.child_spread_factor_pc == 20.0f);

   CHECK(not loaded.child_spread_factor_per_platform);
   CHECK(loaded.child_spread_factor_pc == 20.0f);

   CHECK(not loaded.color_per_platform);
   CHECK(loaded.color_pc == float4{200.0f, 200.0f, 255.0f, 255.0f} / 255.0f);

   CHECK(not loaded.child_color_per_platform);
   CHECK(loaded.child_color_pc == float4{200.0f, 200.0f, 255.0f, 150.0f} / 255.0f);
}

TEST_CASE("world load effects water", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("Water")
{
	PatchDivisions(8, 8);

   OscillationEnable(0);
	OceanEnable(0);
	DisableLowRes();

	WaterRingColor(148, 170, 200, 255);
	WaterWakeColor(200, 200, 200, 255);
	WaterSplashColor((200, 200, 200, 255);


	PC()
	{
		Tile(3.0, 3.0);
		MainTexture("water_pc.tga");
		LODDecimation(1);
		RefractionColor(5, 230, 255, 255);
		ReflectionColor(57, 100, 138, 255);
		UnderwaterColor(61, 128, 144, 128);
		FresnelMinMax(0.1, 0.75);
		FarSceneRange(1500)

		NormalMapTextures("water_pc_normalmap_", 8, 4.0);
		BumpMapTextures("water_pc_bumpmap_", 32, 16.0);
		SpecularMaskTextures("water_pc_specularmask_", 16, 8);
		SpecularMaskTile(4.0, 4.0);
		SpecularMaskScrollSpeed(0.2, 0.2);
		Velocity(0.02, 0.02);
	}

	PS2()
	{
		Tile(1.0, 1.0);
		Velocity(0.00, 0.00);
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
		NormalMapTextures("water_xbox_normalmap_", 4, 16.0);
		LODDecimation(2);
		RefractionColor(110, 135, 139, 255);
		ReflectionColor(110, 135, 139, 255);
		UnderwaterColor(96, 96, 88, 128);
		FresnelMinMax(0.3, 0.6);
		Velocity(0.04, 0.008);
	}
})"sv;

   water loaded = load_effects(world_fx, output).water;

   CHECK(not loaded.ocean_enable_per_platform);
   CHECK(not loaded.ocean_enable_pc);

   CHECK(not loaded.oscillation_enable_per_platform);
   CHECK(not loaded.oscillation_enable_pc);

   CHECK(not loaded.disable_low_res_per_platform);
   CHECK(loaded.disable_low_res_pc);

   CHECK(not loaded.patch_divisions_per_platform);
   CHECK(loaded.patch_divisions_pc == std::array{8, 8});

   CHECK(not loaded.water_ring_color_per_platform);
   CHECK(loaded.water_ring_color_pc == float4{148.0f, 170.0f, 200.0f, 255.0f} / 255.0f);

   CHECK(not loaded.water_wake_color_per_platform);
   CHECK(loaded.water_wake_color_pc == float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f);

   CHECK(not loaded.water_splash_color_per_platform);
   CHECK(loaded.water_splash_color_pc == float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f);

   CHECK(loaded.tile_per_platform);
   CHECK(loaded.tile_pc == float2{3.0f, 3.0f});
   CHECK(loaded.tile_ps2 == float2{1.0f, 1.0f});
   CHECK(loaded.tile_xbox == float2{4.0f, 4.0f});

   CHECK(loaded.main_texture_per_platform);
   CHECK(loaded.main_texture_pc == "water_pc");
   CHECK(loaded.main_texture_ps2 == "water_ps2");
   CHECK(loaded.main_texture_xbox == "");

   CHECK(loaded.lod_decimation_per_platform);
   CHECK(loaded.lod_decimation_pc == 1);
   CHECK(loaded.lod_decimation_ps2 == 8);
   CHECK(loaded.lod_decimation_xbox == 2);

   CHECK(loaded.refraction_color_per_platform);
   CHECK(loaded.refraction_color_pc == float4{5.0f, 230.0f, 255.0f, 255.0f} / 255.0f);
   CHECK(loaded.refraction_color_xbox == float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f);

   CHECK(loaded.reflection_color_per_platform);
   CHECK(loaded.reflection_color_pc == float4{57.0f, 100.0f, 138.0f, 255.0f} / 255.0f);
   CHECK(loaded.reflection_color_xbox == float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f);

   CHECK(loaded.underwater_color_per_platform);
   CHECK(loaded.underwater_color_pc == float4{61.0f, 128.0f, 144.0f, 128.0f} / 255.0f);
   CHECK(loaded.underwater_color_xbox == float4{96.0f, 96.0f, 88.0f, 128.0f} / 255.0f);

   CHECK(loaded.fresnel_min_max_per_platform);
   CHECK(loaded.fresnel_min_max_pc == float2{0.1f, 0.75f});
   CHECK(loaded.fresnel_min_max_xbox == float2{0.3f, 0.6f});

   CHECK(loaded.far_scene_range_pc == 1500.0f);

   CHECK(loaded.normal_map_textures_per_platform);
   CHECK(loaded.normal_map_textures_pc ==
         water::animated_textures{"water_pc_normalmap_", 8, 4.0f});
   CHECK(loaded.normal_map_textures_xbox ==
         water::animated_textures{"water_xbox_normalmap_", 4, 16.0f});

   CHECK(loaded.bump_map_textures_pc ==
         water::animated_textures{"water_pc_bumpmap_", 32, 16.0f});

   CHECK(loaded.specular_mask_textures_pc ==
         water::animated_textures{"water_pc_specularmask_", 16, 8.0f});
   CHECK(loaded.specular_mask_tile_pc == float2{4.0f, 4.0f});
   CHECK(loaded.specular_mask_scroll_speed_pc == float2{0.2f, 0.2f});

   CHECK(loaded.velocity_per_platform);
   CHECK(loaded.velocity_pc == float2{0.02f, 0.02f});
   CHECK(loaded.velocity_ps2 == float2{0.0f, 0.0f});
   CHECK(loaded.velocity_xbox == float2{0.04f, 0.008f});

   CHECK(loaded.min_diffuse_color_ps2 == float4{45.0f, 45.0f, 45.0f, 255.0f} / 255.0f);

   CHECK(loaded.max_diffuse_color_ps2 == float4{85.0f, 85.0f, 85.0f, 255.0f} / 255.0f);

   CHECK(loaded.border_diffuse_color_ps2 == float4{25.0f, 25.0f, 25.0f, 255.0f} / 255.0f);

   CHECK(loaded.specular_color_ps2 == float4{80.0f, 80.0f, 80.0f, 152.0f} / 255.0f);

   CHECK(loaded.speckle_specular_color_ps2 ==
         float4{100.0f, 100.0f, 100.0f, 150.0f} / 255.0f);

   CHECK(loaded.speckle_ambient_color_ps2 == float4{75.0f, 75.0f, 75.0f, 80.0f} / 255.0f);

   CHECK(loaded.speckle_textures_ps2 ==
         water::animated_textures{"water_ps2_specularmask_", 32, 2.0f});

   CHECK(loaded.speckle_tile_ps2 == float2{4.0f, 4.0f});

   CHECK(loaded.speckle_scroll_speed_ps2 == float2{0.1f, 0.1f});

   CHECK(loaded.speckle_coord_shift_ps2 == float2{2.0f, 2.0f});

   CHECK(loaded.light_azim_and_elev_ps2 == float2{0.5f, 0.0f});
}

TEST_CASE("world load effects water ocean", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("Water")
{
	OceanEnable(1);

	FoamTexture("kam1_foam");
	FoamTile(2.0, 2.0);
	WindDirection(0.3, 0.9);
	WindSpeed(32.0);

	PC()
	{
		PhillipsConstant(0.001);
	}

	PS2()
	{
		PhillipsConstant(0.002);
	}

	XBOX()
	{
		PhillipsConstant(0.003);
	}
})"sv;

   water loaded = load_effects(world_fx, output).water;

   CHECK(not loaded.ocean_enable_per_platform);
   CHECK(loaded.ocean_enable_pc);

   CHECK(not loaded.foam_texture_per_platform);
   CHECK(loaded.foam_texture_pc == "kam1_foam");

   CHECK(not loaded.foam_tile_per_platform);
   CHECK(loaded.foam_tile_pc == float2{2.0f, 2.0f});

   CHECK(not loaded.wind_direction_per_platform);
   CHECK(loaded.wind_direction_pc == float2{0.3f, 0.9f});

   CHECK(not loaded.wind_speed_per_platform);
   CHECK(loaded.wind_speed_pc == 32.0f);

   CHECK(loaded.phillips_constant_per_platform);
   CHECK(loaded.phillips_constant_pc == 0.001f);
   CHECK(loaded.phillips_constant_ps2 == 0.002f);
   CHECK(loaded.phillips_constant_xbox == 0.003f);
}

TEST_CASE("world load effects godray", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("Godray")
{
	Enable(1);

	MaxGodraysInWorld(100);
	MaxGodraysOnScreen(4);

	MaxViewDistance(40.0);
	FadeViewDistance(30.0);
	MaxLength(80.0);
	OffsetAngle(-20.0);

	MinRaysPerGodray(2);
	MaxRaysPerGodray(8);
	RadiusForMaxRays(4.0);

	DustVelocity(0.0, -0.1, 0.0);

	Texture("fx_godray");
	TextureScale(1.5, 1.5);
	TextureVelocity(0.0, -0.1, 0.0);
	TextureJitterSpeed(0.1);
})"sv;

   godray loaded = load_effects(world_fx, output).godray;

   CHECK(not loaded.enable_per_platform);
   CHECK(loaded.enable_pc);

   CHECK(not loaded.max_godrays_in_world_per_platform);
   CHECK(loaded.max_godrays_in_world_pc == 100);

   CHECK(not loaded.max_godrays_on_screen_per_platform);
   CHECK(loaded.max_godrays_on_screen_pc == 4);

   CHECK(not loaded.max_view_distance_per_platform);
   CHECK(loaded.max_view_distance_pc == 40.0f);

   CHECK(not loaded.fade_view_distance_per_platform);
   CHECK(loaded.fade_view_distance_pc == 30.0f);

   CHECK(not loaded.max_length_per_platform);
   CHECK(loaded.max_length_pc == 80.0f);

   CHECK(not loaded.offset_angle_per_platform);
   CHECK(loaded.offset_angle_pc == -20.0f);

   CHECK(not loaded.min_rays_per_godray_per_platform);
   CHECK(loaded.min_rays_per_godray_pc == 2);

   CHECK(not loaded.max_rays_per_godray_per_platform);
   CHECK(loaded.max_rays_per_godray_pc == 8);

   CHECK(not loaded.radius_for_max_rays_per_platform);
   CHECK(loaded.radius_for_max_rays_pc == 4.0f);

   CHECK(not loaded.dust_velocity_per_platform);
   CHECK(loaded.dust_velocity_pc == float3{0.0f, -0.1f, 0.0f});

   CHECK(not loaded.texture_per_platform);
   CHECK(loaded.texture_pc == "fx_godray");

   CHECK(not loaded.texture_scale_per_platform);
   CHECK(loaded.texture_scale_pc == float2{1.5f, 1.5f});

   CHECK(not loaded.texture_velocity_per_platform);
   CHECK(loaded.texture_velocity_pc == float3{0.0f, -0.1f, 0.0f});

   CHECK(not loaded.texture_jitter_speed_per_platform);
   CHECK(loaded.texture_jitter_speed_pc == 0.1f);
}

TEST_CASE("world load effects heat shimmer", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("HeatShimmer")
{
	Enable(1);
	WorldHeight(10.0);
	GeometryHeight(4.0);
	ScrollSpeed(0.08);


	PC()
	{
		Tessellation(3);
		BumpMap("shimmer_waves_pc", 1.5, 1.5);
		DistortionScale(0.004);
	}

	PS2()
	{
		Tessellation(20, 40);
		DistortionScale(0.06);
	}

	XBOX()
	{
		Tessellation(4);
		BumpMap("shimmer_waves_xbox", 0.5, 0.5);
		DistortionScale(4.0);
	}
})"sv;

   heat_shimmer loaded = load_effects(world_fx, output).heat_shimmer;

   CHECK(not loaded.enable_per_platform);
   CHECK(loaded.enable_pc);

   CHECK(not loaded.world_height_per_platform);
   CHECK(loaded.world_height_pc == 10.0f);

   CHECK(not loaded.geometry_height_per_platform);
   CHECK(loaded.geometry_height_pc == 4.0f);

   CHECK(not loaded.scroll_speed_per_platform);
   CHECK(loaded.scroll_speed_pc == 0.08f);

   CHECK(loaded.tessellation_pc == 3);
   CHECK(loaded.tessellation_ps2 == std::array{20, 40});
   CHECK(loaded.tessellation_xbox == 4);

   CHECK(loaded.bump_map_per_platform);
   CHECK(loaded.bump_map_pc ==
         heat_shimmer::bump_map{"shimmer_waves_pc", {1.5f, 1.5f}});
   CHECK(loaded.bump_map_xbox ==
         heat_shimmer::bump_map{"shimmer_waves_xbox", {0.5f, 0.5f}});

   CHECK(loaded.distortion_scale_per_platform);
   CHECK(loaded.distortion_scale_pc == 0.004f);
   CHECK(loaded.distortion_scale_ps2 == 0.06f);
   CHECK(loaded.distortion_scale_xbox == 4.0f);
}

TEST_CASE("world load effects space dust", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("SpaceDust")
{
	Enable(1);

	Texture("spacedust");

	SpawnDistance(200.0);
	MaxRandomSideOffset(80.0);
	CenterDeadZoneRadius(40.0);

	MinParticleScale(0.2);
	MaxParticleScale(0.8);

	SpawnDelay(0.2);
	ReferenceSpeed(40.0);

	DustParticleSpeed(50.0);

	SpeedParticleMinLength(4.0);
	SpeedParticleMaxLength(14.0);

	ParticleLengthMinSpeed(70.0);
	ParticleLengthMaxSpeed(200.0);
})"sv;

   space_dust loaded = load_effects(world_fx, output).space_dust;

   CHECK(not loaded.enable_per_platform);
   CHECK(loaded.enable_pc);

   CHECK(not loaded.spawn_distance_per_platform);
   CHECK(loaded.spawn_distance_pc == 200.0f);

   CHECK(not loaded.max_random_side_offset_per_platform);
   CHECK(loaded.max_random_side_offset_pc == 80.0f);

   CHECK(not loaded.dust_particle_speed_per_platform);
   CHECK(loaded.center_dead_zone_radius_pc == 40.0f);

   CHECK(not loaded.min_particle_scale_per_platform);
   CHECK(loaded.min_particle_scale_pc == 0.2f);

   CHECK(not loaded.max_particle_scale_per_platform);
   CHECK(loaded.max_particle_scale_pc == 0.8f);

   CHECK(not loaded.spawn_delay_per_platform);
   CHECK(loaded.spawn_delay_pc == 0.2f);

   CHECK(not loaded.reference_speed_per_platform);
   CHECK(loaded.reference_speed_pc == 40.0f);

   CHECK(not loaded.dust_particle_speed_per_platform);
   CHECK(loaded.dust_particle_speed_pc == 50.0f);

   CHECK(not loaded.speed_particle_min_length_per_platform);
   CHECK(loaded.speed_particle_min_length_pc == 4.0f);

   CHECK(not loaded.speed_particle_max_length_per_platform);
   CHECK(loaded.speed_particle_max_length_pc == 14.0f);

   CHECK(not loaded.particle_length_min_speed_per_platform);
   CHECK(loaded.particle_length_min_speed_pc == 70.0f);

   CHECK(not loaded.particle_length_max_speed_per_platform);
   CHECK(loaded.particle_length_max_speed_pc == 200.0f);
}

TEST_CASE("world load effects world shadow map", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("WorldShadowMap")
{
	Enable(1);
	Texture("shadowy_sun");
	LightName("sun");
	TextureScale(40.0);	
	AnimationFrequency(0.2);
	AnimationAmplitude0(4.0, 0.0);
	AnimationAmplitude1(0.1,-0.1);
})"sv;

   world_shadow_map loaded = load_effects(world_fx, output).world_shadow_map;

   CHECK(not loaded.enable_per_platform);
   CHECK(loaded.enable_pc);

   CHECK(not loaded.texture_per_platform);
   CHECK(loaded.texture_pc == "shadowy_sun");

   CHECK(not loaded.texture_scale_per_platform);
   CHECK(loaded.texture_scale_pc == 40.0f);

   CHECK(not loaded.animation_frequency_per_platform);
   CHECK(loaded.animation_frequency_pc == 0.2f);

   CHECK(not loaded.animation_amplitude0_per_platform);
   CHECK(loaded.animation_amplitude0_pc == float2{4.0f, 0.0f});

   CHECK(not loaded.animation_amplitude1_per_platform);
   CHECK(loaded.animation_amplitude1_pc == float2{0.1f, -0.1f});
}

TEST_CASE("world load effects blur", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("Blur")
{
	PC()
	{
		Enable(1);
		Mode(0)
		ConstantBlend(0.3)
		DownSizeFactor(0.35)
	}
	PS2()
	{
		Enable(1);
		MinMaxDepth(0.95, 1.0);
	}
	XBOX()
	{
		Enable(1);
		Mode(1)
		ConstantBlend(0.45)
		DownSizeFactor(0.5)
	}
})"sv;

   blur loaded = load_effects(world_fx, output).blur;

   CHECK(loaded.enable_per_platform);
   CHECK(loaded.enable_pc);
   CHECK(loaded.enable_ps2);
   CHECK(loaded.enable_xbox);

   CHECK(loaded.constant_blend_per_platform);
   CHECK(loaded.constant_blend_pc == 0.3f);
   CHECK(loaded.constant_blend_ps2 == 0.25f);
   CHECK(loaded.constant_blend_xbox == 0.45f);

   CHECK(loaded.down_size_factor_per_platform);
   CHECK(loaded.down_size_factor_pc == 0.35f);
   CHECK(loaded.down_size_factor_ps2 == 0.25f);
   CHECK(loaded.down_size_factor_xbox == 0.5f);

   CHECK(loaded.min_max_depth_ps2 == float2{0.95f, 1.0f});

   CHECK(loaded.mode_per_platform);
   CHECK(loaded.mode_pc == 0);
   CHECK(loaded.mode_xbox == 1);
}

TEST_CASE("world load effects motion blur", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("MotionBlur")
{
	Enable(0);
})"sv;

   blur loaded = load_effects(world_fx, output).blur;

   CHECK(not loaded.enable_per_platform);
   CHECK(not loaded.enable_pc);
}

TEST_CASE("world load effects scope blur", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("ScopeBlur")
{
	Enable(0);
})"sv;

   blur loaded = load_effects(world_fx, output).blur;

   CHECK(not loaded.enable_per_platform);
   CHECK(not loaded.enable_pc);
}

TEST_CASE("world load effects hdr", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("HDR")
{
	Enable(1);
	DownSizeFactor(0.125);
	NumBloomPasses(3);
	MaxTotalWeight(1.1);
	GlowThreshold(0.75);
	GlowFactor(0.25);
})"sv;

   hdr loaded = load_effects(world_fx, output).hdr;

   CHECK(not loaded.enable_per_platform);
   CHECK(loaded.enable_pc);

   CHECK(not loaded.down_size_factor_per_platform);
   CHECK(loaded.down_size_factor_pc == 0.125f);

   CHECK(not loaded.num_bloom_passes_per_platform);
   CHECK(loaded.num_bloom_passes_pc == 3);

   CHECK(not loaded.max_total_weight_per_platform);
   CHECK(loaded.max_total_weight_pc == 1.1f);

   CHECK(not loaded.glow_threshold_per_platform);
   CHECK(loaded.glow_threshold_pc == 0.75f);

   CHECK(not loaded.glow_factor_per_platform);
   CHECK(loaded.glow_factor_pc == 0.25f);
}

TEST_CASE("world load effects shadow", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
Effect("Shadow")
{
	Enable(1);
	BlurEnable(1);
	Intensity(0.2);
})"sv;

   shadow loaded = load_effects(world_fx, output).shadow;

   CHECK(not loaded.enable_per_platform);
   CHECK(loaded.enable_pc);

   CHECK(not loaded.blur_enable_per_platform);
   CHECK(loaded.blur_enable_pc);

   CHECK(not loaded.intensity_per_platform);
   CHECK(loaded.intensity_pc);
}

TEST_CASE("world load effects sun flare", "[World][IO]")
{
   null_output_stream output;

   const std::string_view world_fx = R"(
SunFlare()
{
	Angle(130.000000, 130.000000);
	Color(255, 150, 150);
	Size(4.0);
   FlareOutSize(5.0);
	InitialFlareOutAlpha(50);
   HaloInnerRing(1.0, 192, 255, 255, 255);
   HaloMiddleRing(4.0, 192, 200, 0, 255);
   HaloOutterRing(5.0, 192, 127, 0, 0);
	SpikeColor(150, 100, 0, 128);
	SpikeSize(10.0);

	PC()
	{
		NumFlareOuts(30);
	}
	PS2()
	{
		NumFlareOuts(40);
	}
	XBOX()
	{
		NumFlareOuts(50);
	}
}

SunFlare()
{	
   Angle(132.000000, 132.000000);
	Color(255, 152, 150);
	Size(6.0);
   FlareOutSize(7.0);
   NumFlareOuts(42);
	InitialFlareOutAlpha(52);
   HaloInnerRing(3.0, 194, 255, 255, 255);
   HaloMiddleRing(6.0, 194, 200, 0, 255);
   HaloOutterRing(7.0, 194, 127, 0, 0);
	SpikeColor(152, 100, 0, 128);
	SpikeSize(12.0);
})"sv;

   std::vector<sun_flare> loaded = load_effects(world_fx, output).sun_flares;

   REQUIRE(loaded.size() == 2);

   CHECK(not loaded[0].angle_per_platform);
   CHECK(loaded[0].angle_pc == float2{130.0f, 130.0f});

   CHECK(not loaded[0].color_per_platform);
   CHECK(loaded[0].color_pc == float3{255.0f, 150.0f, 150.0f} / 255.0f);

   CHECK(not loaded[0].size_per_platform);
   CHECK(loaded[0].size_pc == 4.0f);

   CHECK(not loaded[0].flare_out_size_per_platform);
   CHECK(loaded[0].flare_out_size_pc == 5.0f);

   CHECK(loaded[0].num_flare_outs_per_platform);
   CHECK(loaded[0].num_flare_outs_pc == 30);
   CHECK(loaded[0].num_flare_outs_ps2 == 40);
   CHECK(loaded[0].num_flare_outs_xbox == 50);

   CHECK(not loaded[0].initial_flare_out_alpha_per_platform);
   CHECK(loaded[0].initial_flare_out_alpha_pc == 50);

   using halo_ring = sun_flare::halo_ring;

   CHECK(not loaded[0].halo_inner_ring_per_platform);
   CHECK(loaded[0].halo_inner_ring_pc ==
         halo_ring{1.0f, float4{192.0f, 255.0f, 255.0f, 255.0f} / 255.0f});

   CHECK(not loaded[0].halo_middle_ring_per_platform);
   CHECK(loaded[0].halo_middle_ring_pc ==
         halo_ring{4.0f, float4{192.0f, 200.0f, 0.0f, 255.0f} / 255.0f});

   CHECK(not loaded[0].halo_outter_ring_per_platform);
   CHECK(loaded[0].halo_outter_ring_pc ==
         halo_ring{5.0f, float4{192.0f, 127.0f, 0.0f, 0.0f} / 255.0f});

   CHECK(not loaded[0].spike_color_per_platform);
   CHECK(loaded[0].spike_color_pc == float4{150.0f, 100.0f, 0.0f, 128.0f} / 255.0f);

   CHECK(not loaded[0].spike_size_per_platform);
   CHECK(loaded[0].spike_size_pc == 10.0f);

   CHECK(not loaded[1].angle_per_platform);
   CHECK(loaded[1].angle_pc == float2{132.0f, 132.0f});

   CHECK(not loaded[1].color_per_platform);
   CHECK(loaded[1].color_pc == float3{255.0f, 152.0f, 150.0f} / 255.0f);

   CHECK(not loaded[1].size_per_platform);
   CHECK(loaded[1].size_pc == 6.0f);

   CHECK(not loaded[1].flare_out_size_per_platform);
   CHECK(loaded[1].flare_out_size_pc == 7.0f);

   CHECK(not loaded[1].num_flare_outs_per_platform);
   CHECK(loaded[1].num_flare_outs_pc == 42);

   CHECK(not loaded[1].initial_flare_out_alpha_per_platform);
   CHECK(loaded[1].initial_flare_out_alpha_pc == 52);

   CHECK(not loaded[1].halo_inner_ring_per_platform);
   CHECK(loaded[1].halo_inner_ring_pc ==
         halo_ring{3.0f, float4{194.0f, 255.0f, 255.0f, 255.0f} / 255.0f});

   CHECK(not loaded[1].halo_middle_ring_per_platform);
   CHECK(loaded[1].halo_middle_ring_pc ==
         halo_ring{6.0f, float4{194.0f, 200.0f, 0.0f, 255.0f} / 255.0f});

   CHECK(not loaded[1].halo_outter_ring_per_platform);
   CHECK(loaded[1].halo_outter_ring_pc ==
         halo_ring{7.0f, float4{194.0f, 127.0f, 0.0f, 0.0f} / 255.0f});

   CHECK(not loaded[1].spike_color_per_platform);
   CHECK(loaded[1].spike_color_pc == float4{152.0f, 100.0f, 0.0f, 128.0f} / 255.0f);

   CHECK(not loaded[1].spike_size_per_platform);
   CHECK(loaded[1].spike_size_pc == 12.0f);
}

}
