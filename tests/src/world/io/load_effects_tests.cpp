#include "pch.h"

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

}
