#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/terrain/terrain_io.hpp"
#include "utility/read_file.hpp"

using namespace std::literals;
using namespace Catch::literals;

namespace sk::assets::terrain::tests {

TEST_CASE("terrain io valid tests", "[Assets][Terrain]")
{
   auto terrain = read_terrain(utility::read_file_to_bytes("data/test.ter"sv));

   REQUIRE(terrain.version == version::swbf2);

   REQUIRE(terrain.length == 16);

   CHECK(terrain.height_scale == 0.01_a);
   CHECK(terrain.grid_scale == 4.0_a);

   CHECK(terrain.active_flags.terrain == true);
   CHECK(terrain.active_flags.water == true);
   CHECK(terrain.active_flags.foliage == true);

   CHECK(terrain.water_settings.height == 2.0_a);
   CHECK(terrain.water_settings.u_velocity == 0.0_a);
   CHECK(terrain.water_settings.v_velocity == 0.0_a);
   CHECK(terrain.water_settings.u_repeat == 1.0_a);
   CHECK(terrain.water_settings.v_repeat == 1.0_a);
   CHECK(terrain.water_settings.color.r == Approx(0.00972).margin(0.0025));
   CHECK(terrain.water_settings.color.g == 1.0_a);
   CHECK(terrain.water_settings.color.b == Approx(0.00972).margin(0.0025));
   CHECK(terrain.water_settings.color.a == 1.0_a);
   CHECK(terrain.water_settings.texture == "white.tga"sv);

   CHECK(terrain.texture_names[0] == "white.tga"sv);
   CHECK(terrain.texture_scales[0] == 0.0625_a);
   CHECK(terrain.texture_axes[0] == texture_axis::xy);

   REQUIRE(terrain.height_map.shape()[0] == terrain.length);
   REQUIRE(terrain.height_map.shape()[1] == terrain.length);
   CHECK(terrain.height_map[{0, 0}] == 200);
   CHECK(terrain.height_map[{0, 15}] == 200);
   CHECK(terrain.height_map[{15, 0}] == 200);
   CHECK(terrain.height_map[{15, 15}] == 200);

   REQUIRE(terrain.color_map.shape()[0] == terrain.length);
   REQUIRE(terrain.color_map.shape()[1] == terrain.length);
   CHECK(terrain.color_map[{0, 0}] == 0xff00ff);
   CHECK(terrain.color_map[{0, 15}] == 0xffff00ff);
   CHECK(terrain.color_map[{15, 0}] == 0xff00ff);
   CHECK(terrain.color_map[{15, 15}] == 0xff00ff);

   REQUIRE(terrain.light_map.shape()[0] == terrain.length);
   REQUIRE(terrain.light_map.shape()[1] == terrain.length);

   REQUIRE(terrain.light_map_extra.shape()[0] == terrain.length);
   REQUIRE(terrain.light_map_extra.shape()[1] == terrain.length);
}

}