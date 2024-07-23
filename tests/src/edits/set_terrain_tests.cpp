#include "pch.h"

#include "edits/set_terrain.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

#include <random>

using namespace std::literals;

namespace we::edits::tests {

namespace {

auto make_test_terrain() -> world::terrain
{
   world::terrain terrain{.length = 64};

   for (int16& v : terrain.height_map) v = -1;
   for (uint32& v : terrain.color_map) v = 0xff'00'00'00;
   for (uint8& v : terrain.texture_weight_maps[0]) v = 0xff;

   return terrain;
}

}

TEST_CASE("edits set_terrain", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   const world::terrain test_terrain = make_test_terrain();

   auto edit = make_set_terrain(test_terrain);

   edit->apply(edit_context);

   CHECK(world.terrain.version == test_terrain.version);
   CHECK(world.terrain.length == test_terrain.length);

   CHECK(world.terrain.height_scale == test_terrain.height_scale);
   CHECK(world.terrain.grid_scale == test_terrain.grid_scale);

   CHECK(world.terrain.active_flags == test_terrain.active_flags);
   CHECK(world.terrain.water_settings == test_terrain.water_settings);

   CHECK(world.terrain.texture_names == test_terrain.texture_names);
   CHECK(world.terrain.texture_scales == test_terrain.texture_scales);
   CHECK(world.terrain.texture_axes == test_terrain.texture_axes);
   CHECK(world.terrain.detail_texture_name == test_terrain.detail_texture_name);

   CHECK(world.terrain.height_map == test_terrain.height_map);
   CHECK(world.terrain.color_map == test_terrain.color_map);
   CHECK(world.terrain.light_map == test_terrain.light_map);
   CHECK(world.terrain.light_map_extra == test_terrain.light_map_extra);
   CHECK(world.terrain.texture_weight_maps == test_terrain.texture_weight_maps);
   CHECK(world.terrain.water_map == test_terrain.water_map);
   CHECK(world.terrain.foliage_map == test_terrain.foliage_map);

   const uint32 test_terrain_length = static_cast<uint32>(test_terrain.length);

   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] ==
         world::dirty_rect{0, 0, test_terrain_length, test_terrain_length});

   for (const auto& tracker : world.terrain.texture_weight_maps_dirty) {
      REQUIRE(tracker.size() == 1);
      CHECK(tracker[0] ==
            world::dirty_rect{0, 0, test_terrain_length, test_terrain_length});
   }

   REQUIRE(world.terrain.color_or_light_map_dirty.size() == 1);
   CHECK(world.terrain.color_or_light_map_dirty[0] ==
         world::dirty_rect{0, 0, test_terrain_length, test_terrain_length});

   REQUIRE(world.terrain.water_map_dirty.size() == 1);
   CHECK(world.terrain.water_map_dirty[0] ==
         world::dirty_rect{0, 0, test_terrain_length / 4, test_terrain_length / 4});

   edit->revert(edit_context);

   CHECK(world.terrain.version == test_world.terrain.version);
   CHECK(world.terrain.length == test_world.terrain.length);

   CHECK(world.terrain.height_scale == test_world.terrain.height_scale);
   CHECK(world.terrain.grid_scale == test_world.terrain.grid_scale);

   CHECK(world.terrain.active_flags == test_world.terrain.active_flags);
   CHECK(world.terrain.water_settings == test_world.terrain.water_settings);

   CHECK(world.terrain.texture_names == test_world.terrain.texture_names);
   CHECK(world.terrain.texture_scales == test_world.terrain.texture_scales);
   CHECK(world.terrain.texture_axes == test_world.terrain.texture_axes);
   CHECK(world.terrain.detail_texture_name == test_world.terrain.detail_texture_name);

   CHECK(world.terrain.height_map == test_world.terrain.height_map);
   CHECK(world.terrain.color_map == test_world.terrain.color_map);
   CHECK(world.terrain.light_map == test_world.terrain.light_map);
   CHECK(world.terrain.light_map_extra == test_world.terrain.light_map_extra);
   CHECK(world.terrain.texture_weight_maps == test_world.terrain.texture_weight_maps);
   CHECK(world.terrain.water_map == test_world.terrain.water_map);
   CHECK(world.terrain.foliage_map == test_world.terrain.foliage_map);

   const uint32 test_world_terrain_length =
      static_cast<uint32>(test_world.terrain.length);

   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] ==
         world::dirty_rect{0, 0, test_world_terrain_length, test_world_terrain_length});

   for (const auto& tracker : world.terrain.texture_weight_maps_dirty) {
      REQUIRE(tracker.size() == 1);
      CHECK(tracker[0] == world::dirty_rect{0, 0, test_world_terrain_length,
                                            test_world_terrain_length});
   }

   REQUIRE(world.terrain.color_or_light_map_dirty.size() == 1);
   CHECK(world.terrain.color_or_light_map_dirty[0] ==
         world::dirty_rect{0, 0, test_world_terrain_length, test_world_terrain_length});

   REQUIRE(world.terrain.water_map_dirty.size() == 1);
   CHECK(world.terrain.water_map_dirty[0] ==
         world::dirty_rect{0, 0, test_world_terrain_length / 4,
                           test_world_terrain_length / 4});
}
}
