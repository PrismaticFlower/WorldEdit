#include "pch.h"

#include "assets/sky/io.hpp"

#include <string_view>

using namespace std::literals;

namespace we::assets::sky::tests {

namespace {

const std::string_view sky_test = R"(DomeInfo()
{
	TerrainBumpTexture("terrain_normal_map", 4.0);

	DomeModel()
	{
		Geometry("sky_dome");
	}

	DomeModel()
	{	
		Geometry("sky_clouds");
		rotationspeed(0.002, 0.0, 1.0, 0.0);
	}
	
	PC()
	{	
		DomeModel()
		{	
			Geometry("sky_mountains");
			Offset(1.0);
			MovementScale(0.95);
		}
   }
})";
}

TEST_CASE(".sky reading", "[Assets][REQ]")
{
   auto config = read(sky_test, "PC");

   CHECK(config.terrain_normal_map == "terrain_normal_map");
   CHECK(config.terrain_normal_map_tiling == 4.0f);

   REQUIRE(config.dome_models.size() == 3);

   CHECK(config.dome_models[0].geometry == "sky_dome");
   CHECK(config.dome_models[0].movement_scale == 1.0f);
   CHECK(config.dome_models[0].offset == 0.0f);
   CHECK(config.dome_models[0].rotation.speed == 0.0f);
   CHECK(config.dome_models[0].rotation.direction == float3{0.0f, 1.0f, 0.0f});

   CHECK(config.dome_models[1].geometry == "sky_clouds");
   CHECK(config.dome_models[1].movement_scale == 1.0f);
   CHECK(config.dome_models[1].offset == 0.0f);
   CHECK(config.dome_models[1].rotation.speed == 0.002f);
   CHECK(config.dome_models[1].rotation.direction == float3{0.0f, 1.0f, 0.0f});

   CHECK(config.dome_models[2].geometry == "sky_mountains");
   CHECK(config.dome_models[2].movement_scale == 0.95f);
   CHECK(config.dome_models[2].offset == 1.0f);
   CHECK(config.dome_models[2].rotation.speed == 0.0f);
   CHECK(config.dome_models[2].rotation.direction == float3{0.0f, 1.0f, 0.0f});
}

TEST_CASE(".sky platform drop reading", "[Assets][REQ]")
{
   auto config = read(sky_test, "PS2");

   CHECK(config.terrain_normal_map == "terrain_normal_map");
   CHECK(config.terrain_normal_map_tiling == 4.0f);

   REQUIRE(config.dome_models.size() == 2);

   CHECK(config.dome_models[0].geometry == "sky_dome");
   CHECK(config.dome_models[0].movement_scale == 1.0f);
   CHECK(config.dome_models[0].offset == 0.0f);
   CHECK(config.dome_models[0].rotation.speed == 0.0f);
   CHECK(config.dome_models[0].rotation.direction == float3{0.0f, 1.0f, 0.0f});

   CHECK(config.dome_models[1].geometry == "sky_clouds");
   CHECK(config.dome_models[1].movement_scale == 1.0f);
   CHECK(config.dome_models[1].offset == 0.0f);
   CHECK(config.dome_models[1].rotation.speed == 0.002f);
   CHECK(config.dome_models[1].rotation.direction == float3{0.0f, 1.0f, 0.0f});
}

}
