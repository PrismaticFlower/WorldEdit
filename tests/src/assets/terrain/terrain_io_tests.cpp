#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/terrain/terrain_io.hpp"
#include "io/read_file.hpp"

using namespace std::literals;
using namespace Catch::literals;

namespace we::assets::terrain::tests {

TEST_CASE("terrain io valid tests", "[Assets][Terrain]")
{
   auto terrain = read_terrain(io::read_file_to_bytes("data/test.ter"sv));

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
   CHECK(terrain.water_settings.color.x == Approx(0.00972).margin(0.0025));
   CHECK(terrain.water_settings.color.y == 1.0_a);
   CHECK(terrain.water_settings.color.z == Approx(0.00972).margin(0.0025));
   CHECK(terrain.water_settings.color.w == 1.0_a);
   CHECK(terrain.water_settings.texture == "white"sv);

   CHECK(terrain.texture_names[0] == "white"sv);
   CHECK(terrain.texture_scales[0] == 0.0625_a);
   CHECK(terrain.texture_axes[0] == texture_axis::xy);

   REQUIRE(terrain.height_map.width() == terrain.length);
   REQUIRE(terrain.height_map.height() == terrain.length);
   CHECK(terrain.height_map[{0, 0}] == 200);
   CHECK(terrain.height_map[{0, 15}] == 200);
   CHECK(terrain.height_map[{15, 0}] == 200);
   CHECK(terrain.height_map[{15, 15}] == 200);

   REQUIRE(terrain.color_map.width() == terrain.length);
   REQUIRE(terrain.color_map.height() == terrain.length);
   CHECK(terrain.color_map[{0, 0}] == 0xff00ff);
   CHECK(terrain.color_map[{0, 15}] == 0xffff00ff);
   CHECK(terrain.color_map[{15, 0}] == 0xff00ff);
   CHECK(terrain.color_map[{15, 15}] == 0xff00ff);

   REQUIRE(terrain.light_map.width() == terrain.length);
   REQUIRE(terrain.light_map.height() == terrain.length);

   REQUIRE(terrain.light_map_extra.width() == terrain.length);
   REQUIRE(terrain.light_map_extra.height() == terrain.length);

   REQUIRE(terrain.height_map_dirty.size() == 1);
   CHECK(terrain.height_map_dirty[0] ==
         dirty_rect{0, 0, static_cast<uint32>(terrain.length),
                    static_cast<uint32>(terrain.length)});

   for (const auto& weight_maps_dirty : terrain.texture_weight_maps_dirty) {
      REQUIRE(weight_maps_dirty.size() == 1);
      CHECK(weight_maps_dirty[0] ==
            dirty_rect{0, 0, static_cast<uint32>(terrain.length),
                       static_cast<uint32>(terrain.length)});
   }

   REQUIRE(terrain.color_or_light_map_dirty.size() == 1);
   CHECK(terrain.color_or_light_map_dirty[0] ==
         dirty_rect{0, 0, static_cast<uint32>(terrain.length),
                    static_cast<uint32>(terrain.length)});

   REQUIRE(terrain.water_map_dirty.size() == 1);
   CHECK(terrain.water_map_dirty[0] ==
         dirty_rect{0, 0, static_cast<uint32>(terrain.length / 4),
                    static_cast<uint32>(terrain.length / 4)});
}

}
