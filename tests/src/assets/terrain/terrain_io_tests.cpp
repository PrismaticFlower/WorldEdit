#include "pch.h"

#include "approx_test_helpers.hpp"

#include <assets/terrain/terrain_io.hpp>

#include <utility/read_file.hpp>

using namespace std::literals;
using namespace Catch::literals;

namespace sk::assets::terrain::tests {

TEST_CASE("terrain io valid tests", "[Assets][Terrain]")
{
   auto terrain = read_terrain(utility::read_file_to_bytes("data/test.ter"sv));

   REQUIRE(terrain.version == version::swbf2);

   REQUIRE(terrain.length == 32);
   CHECK(terrain.active_left_offset == -8);
   CHECK(terrain.active_right_offset == 8);
   CHECK(terrain.active_top_offset == -8);
   CHECK(terrain.active_bottom_offset == 8);

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

   REQUIRE(terrain.heightmap.shape()[0] == terrain.length);
   REQUIRE(terrain.heightmap.shape()[1] == terrain.length);
   CHECK(terrain.heightmap[{8, 8}] == 200);
   CHECK(terrain.heightmap[{8, 24}] == 200);
   CHECK(terrain.heightmap[{24, 8}] == 200);
   CHECK(terrain.heightmap[{24, 24}] == 200);

   REQUIRE(terrain.colormap_foreground.shape()[0] == terrain.length);
   REQUIRE(terrain.colormap_foreground.shape()[1] == terrain.length);
   CHECK(terrain.colormap_foreground[{16, 16}].r ==
         Approx(0.13286832155381798).margin(0.0025));
   CHECK(terrain.colormap_foreground[{16, 16}].g ==
         Approx(0.0027401655193954218).margin(0.0025));
   CHECK(terrain.colormap_foreground[{16, 16}].b ==
         Approx(0.19806931955994928).margin(0.0025));
   CHECK(terrain.colormap_foreground[{16, 16}].a == 1.0_a);
   CHECK(terrain.colormap_foreground[{12, 10}].r == 1.0_a);
   CHECK(terrain.colormap_foreground[{12, 10}].g == 1.0_a);
   CHECK(terrain.colormap_foreground[{12, 10}].b == 1.0_a);
   CHECK(terrain.colormap_foreground[{12, 10}].a == 1.0_a);

   REQUIRE(terrain.colormap_background.shape()[0] == terrain.length);
   REQUIRE(terrain.colormap_background.shape()[1] == terrain.length);

   REQUIRE(terrain.lightmap);
   REQUIRE(terrain.lightmap->shape()[0] == terrain.length);
   REQUIRE(terrain.lightmap->shape()[1] == terrain.length);
   CHECK((*terrain.lightmap)[{22, 22}].r == Approx(0.21586050011389882).margin(0.0025));
   CHECK((*terrain.lightmap)[{22, 22}].g == Approx(0.21586050011389882).margin(0.0025));
   CHECK((*terrain.lightmap)[{22, 22}].b == Approx(0.21586050011389882).margin(0.0025));
   CHECK((*terrain.lightmap)[{22, 22}].a == 1.0_a);

   REQUIRE(terrain.texture_weightmap.shape()[0] == terrain.length);
   REQUIRE(terrain.texture_weightmap.shape()[1] == terrain.length);
   CHECK(terrain.texture_weightmap[{15, 20}] == std::array<uint8, 16>{0xff});

   REQUIRE(terrain.water_patches.shape()[0] == terrain.length / terrain.water_patch_size);
   REQUIRE(terrain.water_patches.shape()[1] == terrain.length / terrain.water_patch_size);
   CHECK(terrain.water_patches[{2, 2}] == true);
   CHECK(terrain.water_patches[{0, 0}] == false);

   foliage_patch patch = terrain.foliage_patches[{254, 256}];

   REQUIRE(terrain.foliage_patches.shape()[0] == terrain.foliage_length);
   REQUIRE(terrain.foliage_patches.shape()[1] == terrain.foliage_length);
   CHECK(terrain.foliage_patches[{253, 252}] ==
         foliage_patch{true, false, false, false});
   CHECK(terrain.foliage_patches[{254, 256}] == foliage_patch{true, true, false, true});
}

}