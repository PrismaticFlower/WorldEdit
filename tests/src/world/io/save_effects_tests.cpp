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
}

Effect("HeatShimmer")
{
	Enable(1);
	WorldHeight(10.0);
	GeometryHeight(4.0);
	ScrollSpeed(0.08);

	PS2()
	{
		Tessellation(20, 40);
		DistortionScale(0.06);
	}

	XBOX()
	{
		Tessellation(4, 2);
		BumpMap("shimmer_waves_xbox", 0.5, 0.5);
		DistortionScale(4.0);
	}

	PC()
	{
		Tessellation(3, 2);
		BumpMap("shimmer_waves_pc", 1.5, 1.5);
		DistortionScale(0.004);
	}
}

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
}

Effect("WorldShadowMap")
{
	Enable(1);
	Texture("shadowy_sun");
	LightName("sun");
	TextureScale(40.0);
	AnimationFrequency(0.2);
	AnimationAmplitude0(4.0, 0.0);
	AnimationAmplitude1(0.1, -0.1);
}

Effect("Blur")
{
	Enable(1);

	PS2()
	{
		ConstantBlend(0.25);
		DownSizeFactor(0.25);
		MinMaxDepth(0.95, 1.0);
	}

	XBOX()
	{
		Mode(1);
		ConstantBlend(0.45);
		DownSizeFactor(0.5);
	}

	PC()
	{
		ConstantBlend(0.3);
		DownSizeFactor(0.35);
	}
}

Effect("MotionBlur")
{
	Enable(0);
}

Effect("ScopeBlur")
{
	Enable(0);
}

Effect("HDR")
{
	Enable(1);
	DownSizeFactor(0.125);
	NumBloomPasses(3);
	MaxTotalWeight(1.1);
	GlowThreshold(0.75);
	GlowFactor(0.25);
}

Effect("Shadow")
{
	Enable(1);
	BlurEnable(1);
	Intensity(0.2);
}

SunFlare()
{
	Angle(130.0, 130.0);
	Color(255, 150, 150);
	Size(4.0);
	FlareOutSize(5.0);
	InitialFlareOutAlpha(50);
	HaloInnerRing(1.0, 192, 255, 255, 255);
	HaloMiddleRing(4.0, 192, 200, 0, 255);
	HaloOutterRing(5.0, 192, 127, 0, 0);
	SpikeColor(150, 100, 0, 128);
	SpikeSize(10.0);

	PS2()
	{
		NumFlareOuts(40);
	}

	XBOX()
	{
		NumFlareOuts(50);
	}

	PC()
	{
		NumFlareOuts(30);
	}
}

SunFlare()
{
	Angle(132.0, 132.0);
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
}

)";

   effects effects{
      .color_control =
         {
            .enable = {.pc = true},

            .gamma_brightness = {.pc = 0.6f, .xbox = 0.48f, .per_platform = true},
            .gamma_color_balance = {.pc = 0.55f, .xbox = 0.5f, .per_platform = true},
            .gamma_contrast = {.pc = 0.7f, .xbox = 0.54f, .per_platform = true},
            .gamma_correction = {.pc = 0.5f, .xbox = 0.52f, .per_platform = true},
            .gamma_hue = {.pc = 0.5f, .xbox = 0.52f, .per_platform = true},

            .world_brightness = {0.46f},
            .world_contrast = {0.48f},
            .world_saturation = {0.65f},
         },

      .fog_cloud =
         {
            .enable = {true},
            .texture = {"cloud"},
            .range = {{35.0f, 110.0f}},
            .color = {float4{168.0f, 172.0f, 180.0f, 128.0f} / 255.0f},
            .velocity = {{5.0f, 0.0f}},
            .rotation = {0.1f},
            .height = {24.0f},
            .particle_size = {32.0f},
            .particle_density = {100.0f},
         },

      .wind =
         {
            .enable = {true},
            .velocity = {{3.0f, 0.6f}},
            .velocity_range = {0.75f},
            .velocity_change_rate = {0.2f},
         },

      .precipitation =
         {
            .enable = {true},
            .type = {precipitation_type::streaks},
            .range = {16.0f},
            .color = {float3{200.0f, 200.0f, 228.0f} / 255.0f},
            .velocity = {4.0f},
            .velocity_range = {0.6f},
            .particle_density = {80.0f},
            .particle_density_range = {0.1f},

            .particle_size =
               {
                  .pc = {0.02f},
                  .ps2 = 0.06f,
                  .xbox = 0.03f,
                  .per_platform = true,
               },

            .streak_length = {1.7f},
            .camera_cross_velocity_scale = {0.3f},
            .camera_axial_velocity_scale = {0.9f},
            .ground_effect = {"huge_splash"},
            .ground_effects_per_sec = {7},
            .ground_effect_spread = {16},

            .alpha_min_max =
               {
                  .pc = {0.3f, 0.45f},
                  .ps2 = {0.8f, 1.0f},
                  .xbox = {0.2f, 0.3f},
                  .per_platform = true,
               },

         },

      .lightning =
         {
            .enable = {true},
            .color = {float3{220.0f, 220.0f, 255.0f} / 255.0f},
            .sunlight_fade_factor = {0.2f},
            .sky_dome_darken_factor = {0.3f},
            .brightness_min = {0.7f},
            .fade_time = {0.3f},
            .time_between_flashes_min_max = {{2.0f, 4.0f}},
            .time_between_sub_flashes_min_max = {{0.02f, 0.7f}},
            .num_sub_flashes_min_max = {{1, 8}},
            .horizon_angle_min_max = {{20, 80}},
            .sound_crack = {"test_amb_thunder"},
            .sound_sub_crack = {"test_amb_thundersub"},
         },

      .lightning_bolt =
         {
            .texture = {"lightning2"},
            .width = {10.0f},
            .fade_time = {0.25f},
            .break_distance = {30.0f},
            .texture_size = {40.0f},
            .spread_factor = {30.0f},
            .max_branches = {3.0f},
            .branch_factor = {0.75f},
            .branch_spread_factor = {16},
            .branch_length = {160.0f},
            .interpolation_speed = {0.7f},
            .num_children = {3},
            .child_break_distance = {5.0f},
            .child_texture_size = {4.0f},
            .child_width = {2.0f},
            .child_spread_factor = {20.0f},
            .color = {float4{200.0f, 200.0f, 255.0f, 255.0f} / 255.0f},
            .child_color = {float4{200.0f, 200.0f, 255.0f, 150.0f} / 255.0f},
         },

      .water =
         {
            .ocean_enable = {false},
            .oscillation_enable = {false},
            .disable_low_res = {true},
            .patch_divisions = {{8, 8}},
            .water_ring_color = {float4{148.0f, 170.0f, 200.0f, 255.0f} / 255.0f},
            .water_wake_color = {float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f},
            .water_splash_color = {float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f},

            .lod_decimation = {.pc = 1, .ps2 = 8, .xbox = 2, .per_platform = true},

            .tile =
               {
                  .pc = {3.0f, 3.0f},
                  .ps2 = {1.0f, 1.0f},
                  .xbox = {4.0f, 4.0f},
                  .per_platform = true,
               },

            .velocity =
               {
                  .pc = {0.02f, 0.02f},
                  .ps2 = {0.0f, 0.0f},
                  .xbox = {0.04f, 0.008f},
                  .per_platform = true,
               },

            .main_texture =
               {
                  .pc = "water_pc",
                  .ps2 = "water_ps2",
                  .per_platform = true,
               },

            .refraction_color =
               {
                  .pc = float4{5.0f, 230.0f, 255.0f, 255.0f} / 255.0f,
                  .xbox = float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f,
                  .per_platform = true,
               },

            .reflection_color =
               {
                  .pc = float4{57.0f, 100.0f, 138.0f, 255.0f} / 255.0f,
                  .xbox = float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f,
                  .per_platform = true,
               },

            .underwater_color =
               {
                  .pc = float4{61.0f, 128.0f, 144.0f, 128.0f} / 255.0f,
                  .xbox = float4{96.0f, 96.0f, 88.0f, 128.0f} / 255.0f,
                  .per_platform = true,
               },

            .fresnel_min_max =
               {
                  .pc = {0.1f, 0.75f},
                  .xbox = {0.3f, 0.6f},
                  .per_platform = true,
               },

            .normal_map_textures =
               {
                  .pc = {"water_pc_normalmap_", 8, 4.0f},
                  .xbox = {"water_xbox_normalmap_", 4, 16.0f},
                  .per_platform = true,
               },

            .far_scene_range_pc = {1500.0f},

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

      .godray =
         {
            .enable = {true},
            .max_godrays_in_world = {100},
            .max_godrays_on_screen = {4},
            .max_view_distance = {40.0f},
            .fade_view_distance = {30.0f},
            .max_length = {80.0f},
            .offset_angle = {-20.0f},
            .min_rays_per_godray = {2},
            .max_rays_per_godray = {8},
            .radius_for_max_rays = {4.0f},
            .dust_velocity = {{0.0f, -0.1f, 0.0f}},
            .texture = {"fx_godray"},
            .texture_scale = {{1.5f, 1.5f}},
            .texture_velocity = {{0.0f, -0.1f, 0.0f}},
            .texture_jitter_speed = {0.1f},
         },

      .heat_shimmer =
         {
            .enable = {true},
            .world_height = {10.0f},
            .geometry_height = {4.0f},
            .scroll_speed = {0.08f},
            .bump_map =
               {
                  .pc = {"shimmer_waves_pc", {1.5f, 1.5f}},
                  .xbox = {"shimmer_waves_xbox", {0.5f, 0.5f}},
                  .per_platform = true,
               },
            .distortion_scale =
               {
                  .pc = 0.004f,
                  .ps2 = 0.06f,
                  .xbox = 4.0f,
                  .per_platform = true,
               },
            .tessellation =
               {
                  .pc = {3, 2},
                  .ps2 = {20, 40},
                  .xbox = {4, 2},
                  .per_platform = true,
               },
         },

      .space_dust =
         {
            .enable = {true},
            .texture = {"spacedust"},
            .spawn_distance = {200.0f},
            .max_random_side_offset = {80.0f},
            .center_dead_zone_radius = {40.0f},
            .min_particle_scale = {0.2f},
            .max_particle_scale = {0.8f},
            .spawn_delay = {0.2f},
            .reference_speed = {40.0f},
            .dust_particle_speed = {50.0f},
            .speed_particle_min_length = {4.0f},
            .speed_particle_max_length = {14.0f},
            .particle_length_min_speed = {70.0f},
            .particle_length_max_speed = {200.0f},
         },

      .world_shadow_map =
         {
            .enable = {true},
            .texture = {"shadowy_sun"},
            .light_name = {"sun"},
            .texture_scale = {40.0f},
            .animation_frequency = {0.2f},
            .animation_amplitude0 = {{4.0f, 0.0f}},
            .animation_amplitude1 = {{0.1f, -0.1f}},
         },

      .blur =
         {
            .enable = {true},
            .constant_blend =
               {
                  .pc = 0.3f,
                  .ps2 = 0.25f,
                  .xbox = 0.45f,
                  .per_platform = true,
               },
            .down_size_factor =
               {

                  .pc = 0.35f,
                  .ps2 = 0.25f,
                  .xbox = 0.5f,
                  .per_platform = true,
               },
            .min_max_depth_ps2 = {0.95f, 1.0f},
            .mode =
               {
                  .pc = 0,
                  .xbox = 1,
                  .per_platform = true,
               },
         },

      .motion_blur =
         {
            .enable = {false},
         },

      .scope_blur =
         {
            .enable = {false},
         },

      .hdr =
         {
            .enable = {true},
            .down_size_factor = {0.125f},
            .num_bloom_passes = {3},
            .max_total_weight = {1.1f},
            .glow_threshold = {0.75f},
            .glow_factor = {0.25f},
         },

      .shadow =
         {
            .enable = {true},
            .blur_enable = {true},
            .intensity = {0.2f},
         },

      .sun_flares =
         {pinned_vector_init{.max_size = 2},
          std::initializer_list{
             sun_flare{
                .angle = {{130.0f, 130.0f}},
                .color = {float3{255.0f, 150.0f, 150.0f} / 255.0f},
                .size = {4.0f},
                .flare_out_size = {5.0f},
                .num_flare_outs =
                   {
                      .pc = 30,
                      .ps2 = 40,
                      .xbox = 50,
                      .per_platform = true,
                   },
                .initial_flare_out_alpha = {50},
                .halo_inner_ring = {{1.0f, float4{192.0f, 255.0f, 255.0f, 255.0f} / 255.0f}},
                .halo_middle_ring = {{4.0f, float4{192.0f, 200.0f, 0.0f, 255.0f} / 255.0f}},
                .halo_outter_ring = {{5.0f, float4{192.0f, 127.0f, 0.0f, 0.0f} / 255.0f}},
                .spike_color = {float4{150.0f, 100.0f, 0.0f, 128.0f} / 255.0f},
                .spike_size = {10.0f},
             },
             sun_flare{
                .angle = {{132.0f, 132.0f}},
                .color = {float3{255.0f, 152.0f, 150.0f} / 255.0f},
                .size = {6.0f},
                .flare_out_size = {7.0f},
                .num_flare_outs = {42},
                .initial_flare_out_alpha = {52},
                .halo_inner_ring = {{3.0f, float4{194.0f, 255.0f, 255.0f, 255.0f} / 255.0f}},
                .halo_middle_ring = {{6.0f, float4{194.0f, 200.0f, 0.0f, 255.0f} / 255.0f}},
                .halo_outter_ring = {{7.0f, float4{194.0f, 127.0f, 0.0f, 0.0f} / 255.0f}},
                .spike_color = {float4{152.0f, 100.0f, 0.0f, 128.0f} / 255.0f},
                .spike_size = {12.0f},
             },
          }},
   };

   (void)io::create_directory("temp/world");

   save_effects("temp/world/test.fx", effects);

   const auto written_fx = io::read_file_to_string("temp/world/test.fx");

   CHECK(written_fx == expected_fx);
}

TEST_CASE("world save effects (no wind)", "[World][IO]")
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
}

Effect("HeatShimmer")
{
	Enable(1);
	WorldHeight(10.0);
	GeometryHeight(4.0);
	ScrollSpeed(0.08);

	PS2()
	{
		Tessellation(20, 40);
		DistortionScale(0.06);
	}

	XBOX()
	{
		Tessellation(4, 2);
		BumpMap("shimmer_waves_xbox", 0.5, 0.5);
		DistortionScale(4.0);
	}

	PC()
	{
		Tessellation(3, 2);
		BumpMap("shimmer_waves_pc", 1.5, 1.5);
		DistortionScale(0.004);
	}
}

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
}

Effect("WorldShadowMap")
{
	Enable(1);
	Texture("shadowy_sun");
	LightName("sun");
	TextureScale(40.0);
	AnimationFrequency(0.2);
	AnimationAmplitude0(4.0, 0.0);
	AnimationAmplitude1(0.1, -0.1);
}

Effect("Blur")
{
	Enable(1);

	PS2()
	{
		ConstantBlend(0.25);
		DownSizeFactor(0.25);
		MinMaxDepth(0.95, 1.0);
	}

	XBOX()
	{
		Mode(1);
		ConstantBlend(0.45);
		DownSizeFactor(0.5);
	}

	PC()
	{
		ConstantBlend(0.3);
		DownSizeFactor(0.35);
	}
}

Effect("MotionBlur")
{
	Enable(0);
}

Effect("ScopeBlur")
{
	Enable(0);
}

Effect("HDR")
{
	Enable(1);
	DownSizeFactor(0.125);
	NumBloomPasses(3);
	MaxTotalWeight(1.1);
	GlowThreshold(0.75);
	GlowFactor(0.25);
}

Effect("Shadow")
{
	Enable(1);
	BlurEnable(1);
	Intensity(0.2);
}

SunFlare()
{
	Angle(130.0, 130.0);
	Color(255, 150, 150);
	Size(4.0);
	FlareOutSize(5.0);
	InitialFlareOutAlpha(50);
	HaloInnerRing(1.0, 192, 255, 255, 255);
	HaloMiddleRing(4.0, 192, 200, 0, 255);
	HaloOutterRing(5.0, 192, 127, 0, 0);
	SpikeColor(150, 100, 0, 128);
	SpikeSize(10.0);

	PS2()
	{
		NumFlareOuts(40);
	}

	XBOX()
	{
		NumFlareOuts(50);
	}

	PC()
	{
		NumFlareOuts(30);
	}
}

SunFlare()
{
	Angle(132.0, 132.0);
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
}

)";

   effects effects{
      .color_control =
         {
            .enable = {.pc = true},

            .gamma_brightness = {.pc = 0.6f, .xbox = 0.48f, .per_platform = true},
            .gamma_color_balance = {.pc = 0.55f, .xbox = 0.5f, .per_platform = true},
            .gamma_contrast = {.pc = 0.7f, .xbox = 0.54f, .per_platform = true},
            .gamma_correction = {.pc = 0.5f, .xbox = 0.52f, .per_platform = true},
            .gamma_hue = {.pc = 0.5f, .xbox = 0.52f, .per_platform = true},

            .world_brightness = {0.46f},
            .world_contrast = {0.48f},
            .world_saturation = {0.65f},
         },

      .fog_cloud =
         {
            .enable = {true},
            .texture = {"cloud"},
            .range = {{35.0f, 110.0f}},
            .color = {float4{168.0f, 172.0f, 180.0f, 128.0f} / 255.0f},
            .velocity = {{5.0f, 0.0f}},
            .rotation = {0.1f},
            .height = {24.0f},
            .particle_size = {32.0f},
            .particle_density = {100.0f},
         },

      .precipitation =
         {
            .enable = {true},
            .type = {precipitation_type::streaks},
            .range = {16.0f},
            .color = {float3{200.0f, 200.0f, 228.0f} / 255.0f},
            .velocity = {4.0f},
            .velocity_range = {0.6f},
            .particle_density = {80.0f},
            .particle_density_range = {0.1f},

            .particle_size =
               {
                  .pc = {0.02f},
                  .ps2 = 0.06f,
                  .xbox = 0.03f,
                  .per_platform = true,
               },

            .streak_length = {1.7f},
            .camera_cross_velocity_scale = {0.3f},
            .camera_axial_velocity_scale = {0.9f},
            .ground_effect = {"huge_splash"},
            .ground_effects_per_sec = {7},
            .ground_effect_spread = {16},

            .alpha_min_max =
               {
                  .pc = {0.3f, 0.45f},
                  .ps2 = {0.8f, 1.0f},
                  .xbox = {0.2f, 0.3f},
                  .per_platform = true,
               },

         },

      .lightning =
         {
            .enable = {true},
            .color = {float3{220.0f, 220.0f, 255.0f} / 255.0f},
            .sunlight_fade_factor = {0.2f},
            .sky_dome_darken_factor = {0.3f},
            .brightness_min = {0.7f},
            .fade_time = {0.3f},
            .time_between_flashes_min_max = {{2.0f, 4.0f}},
            .time_between_sub_flashes_min_max = {{0.02f, 0.7f}},
            .num_sub_flashes_min_max = {{1, 8}},
            .horizon_angle_min_max = {{20, 80}},
            .sound_crack = {"test_amb_thunder"},
            .sound_sub_crack = {"test_amb_thundersub"},
         },

      .lightning_bolt =
         {
            .texture = {"lightning2"},
            .width = {10.0f},
            .fade_time = {0.25f},
            .break_distance = {30.0f},
            .texture_size = {40.0f},
            .spread_factor = {30.0f},
            .max_branches = {3.0f},
            .branch_factor = {0.75f},
            .branch_spread_factor = {16},
            .branch_length = {160.0f},
            .interpolation_speed = {0.7f},
            .num_children = {3},
            .child_break_distance = {5.0f},
            .child_texture_size = {4.0f},
            .child_width = {2.0f},
            .child_spread_factor = {20.0f},
            .color = {float4{200.0f, 200.0f, 255.0f, 255.0f} / 255.0f},
            .child_color = {float4{200.0f, 200.0f, 255.0f, 150.0f} / 255.0f},
         },

      .water =
         {
            .ocean_enable = {false},
            .oscillation_enable = {false},
            .disable_low_res = {true},
            .patch_divisions = {{8, 8}},
            .water_ring_color = {float4{148.0f, 170.0f, 200.0f, 255.0f} / 255.0f},
            .water_wake_color = {float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f},
            .water_splash_color = {float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f},

            .lod_decimation = {.pc = 1, .ps2 = 8, .xbox = 2, .per_platform = true},

            .tile =
               {
                  .pc = {3.0f, 3.0f},
                  .ps2 = {1.0f, 1.0f},
                  .xbox = {4.0f, 4.0f},
                  .per_platform = true,
               },

            .velocity =
               {
                  .pc = {0.02f, 0.02f},
                  .ps2 = {0.0f, 0.0f},
                  .xbox = {0.04f, 0.008f},
                  .per_platform = true,
               },

            .main_texture =
               {
                  .pc = "water_pc",
                  .ps2 = "water_ps2",
                  .per_platform = true,
               },

            .refraction_color =
               {
                  .pc = float4{5.0f, 230.0f, 255.0f, 255.0f} / 255.0f,
                  .xbox = float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f,
                  .per_platform = true,
               },

            .reflection_color =
               {
                  .pc = float4{57.0f, 100.0f, 138.0f, 255.0f} / 255.0f,
                  .xbox = float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f,
                  .per_platform = true,
               },

            .underwater_color =
               {
                  .pc = float4{61.0f, 128.0f, 144.0f, 128.0f} / 255.0f,
                  .xbox = float4{96.0f, 96.0f, 88.0f, 128.0f} / 255.0f,
                  .per_platform = true,
               },

            .fresnel_min_max =
               {
                  .pc = {0.1f, 0.75f},
                  .xbox = {0.3f, 0.6f},
                  .per_platform = true,
               },

            .normal_map_textures =
               {
                  .pc = {"water_pc_normalmap_", 8, 4.0f},
                  .xbox = {"water_xbox_normalmap_", 4, 16.0f},
                  .per_platform = true,
               },

            .far_scene_range_pc = {1500.0f},

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

      .godray =
         {
            .enable = {true},
            .max_godrays_in_world = {100},
            .max_godrays_on_screen = {4},
            .max_view_distance = {40.0f},
            .fade_view_distance = {30.0f},
            .max_length = {80.0f},
            .offset_angle = {-20.0f},
            .min_rays_per_godray = {2},
            .max_rays_per_godray = {8},
            .radius_for_max_rays = {4.0f},
            .dust_velocity = {{0.0f, -0.1f, 0.0f}},
            .texture = {"fx_godray"},
            .texture_scale = {{1.5f, 1.5f}},
            .texture_velocity = {{0.0f, -0.1f, 0.0f}},
            .texture_jitter_speed = {0.1f},
         },

      .heat_shimmer =
         {
            .enable = {true},
            .world_height = {10.0f},
            .geometry_height = {4.0f},
            .scroll_speed = {0.08f},
            .bump_map =
               {
                  .pc = {"shimmer_waves_pc", {1.5f, 1.5f}},
                  .xbox = {"shimmer_waves_xbox", {0.5f, 0.5f}},
                  .per_platform = true,
               },
            .distortion_scale =
               {
                  .pc = 0.004f,
                  .ps2 = 0.06f,
                  .xbox = 4.0f,
                  .per_platform = true,
               },
            .tessellation =
               {
                  .pc = {3, 2},
                  .ps2 = {20, 40},
                  .xbox = {4, 2},
                  .per_platform = true,
               },
         },

      .space_dust =
         {
            .enable = {true},
            .texture = {"spacedust"},
            .spawn_distance = {200.0f},
            .max_random_side_offset = {80.0f},
            .center_dead_zone_radius = {40.0f},
            .min_particle_scale = {0.2f},
            .max_particle_scale = {0.8f},
            .spawn_delay = {0.2f},
            .reference_speed = {40.0f},
            .dust_particle_speed = {50.0f},
            .speed_particle_min_length = {4.0f},
            .speed_particle_max_length = {14.0f},
            .particle_length_min_speed = {70.0f},
            .particle_length_max_speed = {200.0f},
         },

      .world_shadow_map =
         {
            .enable = {true},
            .texture = {"shadowy_sun"},
            .light_name = {"sun"},
            .texture_scale = {40.0f},
            .animation_frequency = {0.2f},
            .animation_amplitude0 = {{4.0f, 0.0f}},
            .animation_amplitude1 = {{0.1f, -0.1f}},
         },

      .blur =
         {
            .enable = {true},
            .constant_blend =
               {
                  .pc = 0.3f,
                  .ps2 = 0.25f,
                  .xbox = 0.45f,
                  .per_platform = true,
               },
            .down_size_factor =
               {

                  .pc = 0.35f,
                  .ps2 = 0.25f,
                  .xbox = 0.5f,
                  .per_platform = true,
               },
            .min_max_depth_ps2 = {0.95f, 1.0f},
            .mode =
               {
                  .pc = 0,
                  .xbox = 1,
                  .per_platform = true,
               },
         },

      .motion_blur =
         {
            .enable = {false},
         },

      .scope_blur =
         {
            .enable = {false},
         },

      .hdr =
         {
            .enable = {true},
            .down_size_factor = {0.125f},
            .num_bloom_passes = {3},
            .max_total_weight = {1.1f},
            .glow_threshold = {0.75f},
            .glow_factor = {0.25f},
         },

      .shadow =
         {
            .enable = {true},
            .blur_enable = {true},
            .intensity = {0.2f},
         },

      .sun_flares =
         {pinned_vector_init{.max_size = 2},
          std::initializer_list{
             sun_flare{
                .angle = {{130.0f, 130.0f}},
                .color = {float3{255.0f, 150.0f, 150.0f} / 255.0f},
                .size = {4.0f},
                .flare_out_size = {5.0f},
                .num_flare_outs =
                   {
                      .pc = 30,
                      .ps2 = 40,
                      .xbox = 50,
                      .per_platform = true,
                   },
                .initial_flare_out_alpha = {50},
                .halo_inner_ring = {{1.0f, float4{192.0f, 255.0f, 255.0f, 255.0f} / 255.0f}},
                .halo_middle_ring = {{4.0f, float4{192.0f, 200.0f, 0.0f, 255.0f} / 255.0f}},
                .halo_outter_ring = {{5.0f, float4{192.0f, 127.0f, 0.0f, 0.0f} / 255.0f}},
                .spike_color = {float4{150.0f, 100.0f, 0.0f, 128.0f} / 255.0f},
                .spike_size = {10.0f},
             },
             sun_flare{
                .angle = {{132.0f, 132.0f}},
                .color = {float3{255.0f, 152.0f, 150.0f} / 255.0f},
                .size = {6.0f},
                .flare_out_size = {7.0f},
                .num_flare_outs = {42},
                .initial_flare_out_alpha = {52},
                .halo_inner_ring = {{3.0f, float4{194.0f, 255.0f, 255.0f, 255.0f} / 255.0f}},
                .halo_middle_ring = {{6.0f, float4{194.0f, 200.0f, 0.0f, 255.0f} / 255.0f}},
                .halo_outter_ring = {{7.0f, float4{194.0f, 127.0f, 0.0f, 0.0f} / 255.0f}},
                .spike_color = {float4{152.0f, 100.0f, 0.0f, 128.0f} / 255.0f},
                .spike_size = {12.0f},
             },
          }},
   };

   (void)io::create_directory("temp/world");

   save_effects("temp/world/test_no_wind.fx", effects);

   const auto written_fx =
      io::read_file_to_string("temp/world/test_no_wind.fx");

   CHECK(written_fx == expected_fx);
}

TEST_CASE("world save effects (no ground splash)", "[World][IO]")
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
}

Effect("HeatShimmer")
{
	Enable(1);
	WorldHeight(10.0);
	GeometryHeight(4.0);
	ScrollSpeed(0.08);

	PS2()
	{
		Tessellation(20, 40);
		DistortionScale(0.06);
	}

	XBOX()
	{
		Tessellation(4, 2);
		BumpMap("shimmer_waves_xbox", 0.5, 0.5);
		DistortionScale(4.0);
	}

	PC()
	{
		Tessellation(3, 2);
		BumpMap("shimmer_waves_pc", 1.5, 1.5);
		DistortionScale(0.004);
	}
}

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
}

Effect("WorldShadowMap")
{
	Enable(1);
	Texture("shadowy_sun");
	LightName("sun");
	TextureScale(40.0);
	AnimationFrequency(0.2);
	AnimationAmplitude0(4.0, 0.0);
	AnimationAmplitude1(0.1, -0.1);
}

Effect("Blur")
{
	Enable(1);

	PS2()
	{
		ConstantBlend(0.25);
		DownSizeFactor(0.25);
		MinMaxDepth(0.95, 1.0);
	}

	XBOX()
	{
		Mode(1);
		ConstantBlend(0.45);
		DownSizeFactor(0.5);
	}

	PC()
	{
		ConstantBlend(0.3);
		DownSizeFactor(0.35);
	}
}

Effect("MotionBlur")
{
	Enable(0);
}

Effect("ScopeBlur")
{
	Enable(0);
}

Effect("HDR")
{
	Enable(1);
	DownSizeFactor(0.125);
	NumBloomPasses(3);
	MaxTotalWeight(1.1);
	GlowThreshold(0.75);
	GlowFactor(0.25);
}

Effect("Shadow")
{
	Enable(1);
	BlurEnable(1);
	Intensity(0.2);
}

SunFlare()
{
	Angle(130.0, 130.0);
	Color(255, 150, 150);
	Size(4.0);
	FlareOutSize(5.0);
	InitialFlareOutAlpha(50);
	HaloInnerRing(1.0, 192, 255, 255, 255);
	HaloMiddleRing(4.0, 192, 200, 0, 255);
	HaloOutterRing(5.0, 192, 127, 0, 0);
	SpikeColor(150, 100, 0, 128);
	SpikeSize(10.0);

	PS2()
	{
		NumFlareOuts(40);
	}

	XBOX()
	{
		NumFlareOuts(50);
	}

	PC()
	{
		NumFlareOuts(30);
	}
}

SunFlare()
{
	Angle(132.0, 132.0);
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
}

)";

   effects effects{
      .color_control =
         {
            .enable = {.pc = true},

            .gamma_brightness = {.pc = 0.6f, .xbox = 0.48f, .per_platform = true},
            .gamma_color_balance = {.pc = 0.55f, .xbox = 0.5f, .per_platform = true},
            .gamma_contrast = {.pc = 0.7f, .xbox = 0.54f, .per_platform = true},
            .gamma_correction = {.pc = 0.5f, .xbox = 0.52f, .per_platform = true},
            .gamma_hue = {.pc = 0.5f, .xbox = 0.52f, .per_platform = true},

            .world_brightness = {0.46f},
            .world_contrast = {0.48f},
            .world_saturation = {0.65f},
         },

      .fog_cloud =
         {
            .enable = {true},
            .texture = {"cloud"},
            .range = {{35.0f, 110.0f}},
            .color = {float4{168.0f, 172.0f, 180.0f, 128.0f} / 255.0f},
            .velocity = {{5.0f, 0.0f}},
            .rotation = {0.1f},
            .height = {24.0f},
            .particle_size = {32.0f},
            .particle_density = {100.0f},
         },

      .wind =
         {
            .enable = {true},
            .velocity = {{3.0f, 0.6f}},
            .velocity_range = {0.75f},
            .velocity_change_rate = {0.2f},
         },

      .precipitation =
         {
            .enable = {true},
            .type = {precipitation_type::streaks},
            .range = {16.0f},
            .color = {float3{200.0f, 200.0f, 228.0f} / 255.0f},
            .velocity = {4.0f},
            .velocity_range = {0.6f},
            .particle_density = {80.0f},
            .particle_density_range = {0.1f},

            .particle_size =
               {
                  .pc = {0.02f},
                  .ps2 = 0.06f,
                  .xbox = 0.03f,
                  .per_platform = true,
               },

            .streak_length = {1.7f},
            .camera_cross_velocity_scale = {0.3f},
            .camera_axial_velocity_scale = {0.9f},
            .ground_effects_per_sec = {7},
            .ground_effect_spread = {16},

            .alpha_min_max =
               {
                  .pc = {0.3f, 0.45f},
                  .ps2 = {0.8f, 1.0f},
                  .xbox = {0.2f, 0.3f},
                  .per_platform = true,
               },

         },

      .lightning =
         {
            .enable = {true},
            .color = {float3{220.0f, 220.0f, 255.0f} / 255.0f},
            .sunlight_fade_factor = {0.2f},
            .sky_dome_darken_factor = {0.3f},
            .brightness_min = {0.7f},
            .fade_time = {0.3f},
            .time_between_flashes_min_max = {{2.0f, 4.0f}},
            .time_between_sub_flashes_min_max = {{0.02f, 0.7f}},
            .num_sub_flashes_min_max = {{1, 8}},
            .horizon_angle_min_max = {{20, 80}},
            .sound_crack = {"test_amb_thunder"},
            .sound_sub_crack = {"test_amb_thundersub"},
         },

      .lightning_bolt =
         {
            .texture = {"lightning2"},
            .width = {10.0f},
            .fade_time = {0.25f},
            .break_distance = {30.0f},
            .texture_size = {40.0f},
            .spread_factor = {30.0f},
            .max_branches = {3.0f},
            .branch_factor = {0.75f},
            .branch_spread_factor = {16},
            .branch_length = {160.0f},
            .interpolation_speed = {0.7f},
            .num_children = {3},
            .child_break_distance = {5.0f},
            .child_texture_size = {4.0f},
            .child_width = {2.0f},
            .child_spread_factor = {20.0f},
            .color = {float4{200.0f, 200.0f, 255.0f, 255.0f} / 255.0f},
            .child_color = {float4{200.0f, 200.0f, 255.0f, 150.0f} / 255.0f},
         },

      .water =
         {
            .ocean_enable = {false},
            .oscillation_enable = {false},
            .disable_low_res = {true},
            .patch_divisions = {{8, 8}},
            .water_ring_color = {float4{148.0f, 170.0f, 200.0f, 255.0f} / 255.0f},
            .water_wake_color = {float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f},
            .water_splash_color = {float4{200.0f, 200.0f, 200.0f, 255.0f} / 255.0f},

            .lod_decimation = {.pc = 1, .ps2 = 8, .xbox = 2, .per_platform = true},

            .tile =
               {
                  .pc = {3.0f, 3.0f},
                  .ps2 = {1.0f, 1.0f},
                  .xbox = {4.0f, 4.0f},
                  .per_platform = true,
               },

            .velocity =
               {
                  .pc = {0.02f, 0.02f},
                  .ps2 = {0.0f, 0.0f},
                  .xbox = {0.04f, 0.008f},
                  .per_platform = true,
               },

            .main_texture =
               {
                  .pc = "water_pc",
                  .ps2 = "water_ps2",
                  .per_platform = true,
               },

            .refraction_color =
               {
                  .pc = float4{5.0f, 230.0f, 255.0f, 255.0f} / 255.0f,
                  .xbox = float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f,
                  .per_platform = true,
               },

            .reflection_color =
               {
                  .pc = float4{57.0f, 100.0f, 138.0f, 255.0f} / 255.0f,
                  .xbox = float4{110.0f, 135.0f, 139.0f, 255.0f} / 255.0f,
                  .per_platform = true,
               },

            .underwater_color =
               {
                  .pc = float4{61.0f, 128.0f, 144.0f, 128.0f} / 255.0f,
                  .xbox = float4{96.0f, 96.0f, 88.0f, 128.0f} / 255.0f,
                  .per_platform = true,
               },

            .fresnel_min_max =
               {
                  .pc = {0.1f, 0.75f},
                  .xbox = {0.3f, 0.6f},
                  .per_platform = true,
               },

            .normal_map_textures =
               {
                  .pc = {"water_pc_normalmap_", 8, 4.0f},
                  .xbox = {"water_xbox_normalmap_", 4, 16.0f},
                  .per_platform = true,
               },

            .far_scene_range_pc = {1500.0f},

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

      .godray =
         {
            .enable = {true},
            .max_godrays_in_world = {100},
            .max_godrays_on_screen = {4},
            .max_view_distance = {40.0f},
            .fade_view_distance = {30.0f},
            .max_length = {80.0f},
            .offset_angle = {-20.0f},
            .min_rays_per_godray = {2},
            .max_rays_per_godray = {8},
            .radius_for_max_rays = {4.0f},
            .dust_velocity = {{0.0f, -0.1f, 0.0f}},
            .texture = {"fx_godray"},
            .texture_scale = {{1.5f, 1.5f}},
            .texture_velocity = {{0.0f, -0.1f, 0.0f}},
            .texture_jitter_speed = {0.1f},
         },

      .heat_shimmer =
         {
            .enable = {true},
            .world_height = {10.0f},
            .geometry_height = {4.0f},
            .scroll_speed = {0.08f},
            .bump_map =
               {
                  .pc = {"shimmer_waves_pc", {1.5f, 1.5f}},
                  .xbox = {"shimmer_waves_xbox", {0.5f, 0.5f}},
                  .per_platform = true,
               },
            .distortion_scale =
               {
                  .pc = 0.004f,
                  .ps2 = 0.06f,
                  .xbox = 4.0f,
                  .per_platform = true,
               },
            .tessellation =
               {
                  .pc = {3, 2},
                  .ps2 = {20, 40},
                  .xbox = {4, 2},
                  .per_platform = true,
               },
         },

      .space_dust =
         {
            .enable = {true},
            .texture = {"spacedust"},
            .spawn_distance = {200.0f},
            .max_random_side_offset = {80.0f},
            .center_dead_zone_radius = {40.0f},
            .min_particle_scale = {0.2f},
            .max_particle_scale = {0.8f},
            .spawn_delay = {0.2f},
            .reference_speed = {40.0f},
            .dust_particle_speed = {50.0f},
            .speed_particle_min_length = {4.0f},
            .speed_particle_max_length = {14.0f},
            .particle_length_min_speed = {70.0f},
            .particle_length_max_speed = {200.0f},
         },

      .world_shadow_map =
         {
            .enable = {true},
            .texture = {"shadowy_sun"},
            .light_name = {"sun"},
            .texture_scale = {40.0f},
            .animation_frequency = {0.2f},
            .animation_amplitude0 = {{4.0f, 0.0f}},
            .animation_amplitude1 = {{0.1f, -0.1f}},
         },

      .blur =
         {
            .enable = {true},
            .constant_blend =
               {
                  .pc = 0.3f,
                  .ps2 = 0.25f,
                  .xbox = 0.45f,
                  .per_platform = true,
               },
            .down_size_factor =
               {

                  .pc = 0.35f,
                  .ps2 = 0.25f,
                  .xbox = 0.5f,
                  .per_platform = true,
               },
            .min_max_depth_ps2 = {0.95f, 1.0f},
            .mode =
               {
                  .pc = 0,
                  .xbox = 1,
                  .per_platform = true,
               },
         },

      .motion_blur =
         {
            .enable = {false},
         },

      .scope_blur =
         {
            .enable = {false},
         },

      .hdr =
         {
            .enable = {true},
            .down_size_factor = {0.125f},
            .num_bloom_passes = {3},
            .max_total_weight = {1.1f},
            .glow_threshold = {0.75f},
            .glow_factor = {0.25f},
         },

      .shadow =
         {
            .enable = {true},
            .blur_enable = {true},
            .intensity = {0.2f},
         },

      .sun_flares =
         {pinned_vector_init{.max_size = 2},
          std::initializer_list{
             sun_flare{
                .angle = {{130.0f, 130.0f}},
                .color = {float3{255.0f, 150.0f, 150.0f} / 255.0f},
                .size = {4.0f},
                .flare_out_size = {5.0f},
                .num_flare_outs =
                   {
                      .pc = 30,
                      .ps2 = 40,
                      .xbox = 50,
                      .per_platform = true,
                   },
                .initial_flare_out_alpha = {50},
                .halo_inner_ring = {{1.0f, float4{192.0f, 255.0f, 255.0f, 255.0f} / 255.0f}},
                .halo_middle_ring = {{4.0f, float4{192.0f, 200.0f, 0.0f, 255.0f} / 255.0f}},
                .halo_outter_ring = {{5.0f, float4{192.0f, 127.0f, 0.0f, 0.0f} / 255.0f}},
                .spike_color = {float4{150.0f, 100.0f, 0.0f, 128.0f} / 255.0f},
                .spike_size = {10.0f},
             },
             sun_flare{
                .angle = {{132.0f, 132.0f}},
                .color = {float3{255.0f, 152.0f, 150.0f} / 255.0f},
                .size = {6.0f},
                .flare_out_size = {7.0f},
                .num_flare_outs = {42},
                .initial_flare_out_alpha = {52},
                .halo_inner_ring = {{3.0f, float4{194.0f, 255.0f, 255.0f, 255.0f} / 255.0f}},
                .halo_middle_ring = {{6.0f, float4{194.0f, 200.0f, 0.0f, 255.0f} / 255.0f}},
                .halo_outter_ring = {{7.0f, float4{194.0f, 127.0f, 0.0f, 0.0f} / 255.0f}},
                .spike_color = {float4{152.0f, 100.0f, 0.0f, 128.0f} / 255.0f},
                .spike_size = {12.0f},
             },
          }},
   };

   (void)io::create_directory("temp/world");

   save_effects("temp/world/test.fx", effects);

   const auto written_fx = io::read_file_to_string("temp/world/test.fx");

   CHECK(written_fx == expected_fx);
}

}