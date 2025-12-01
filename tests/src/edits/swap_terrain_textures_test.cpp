#include "pch.h"

#include "edits/swap_terrain_textures.hpp"
#include "world/world.hpp"

using namespace std::literals;
using we::world::dirty_rect;

namespace we::edits::tests {

namespace {

bool check_map(const container::dynamic_array_2d<uint8>& map, const uint8 expected) noexcept
{
   for (const uint8 v : map) {
      if (v != expected) return false;
   }

   return true;
}

}

TEST_CASE("edits swap_terrain_textures", "[Edits]")
{
   world::world world{.terrain = {.length = 32}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world.terrain.texture_names[1] = "dirt";
   world.terrain.texture_names[3] = "snow";

   world.terrain.texture_scales[1] = 2.0f;
   world.terrain.texture_scales[3] = 3.0f;

   world.terrain.texture_axes[1] = world::texture_axis::yx;
   world.terrain.texture_axes[3] = world::texture_axis::negative_yx;

   for (uint8& v : world.terrain.texture_weight_maps[1]) v = 0x1;
   for (uint8& v : world.terrain.texture_weight_maps[3]) v = 0x3;

   auto edit = make_swap_terrain_textures(1, 3);

   edit->apply(edit_context);

   CHECK(world.terrain.texture_names[1] == "snow");
   CHECK(world.terrain.texture_names[3] == "dirt");

   CHECK(world.terrain.texture_scales[1] == 3.0f);
   CHECK(world.terrain.texture_scales[3] == 2.0f);

   CHECK(world.terrain.texture_axes[1] == world::texture_axis::negative_yx);
   CHECK(world.terrain.texture_axes[3] == world::texture_axis::yx);

   CHECK(check_map(world.terrain.texture_weight_maps[1], 0x3));
   CHECK(check_map(world.terrain.texture_weight_maps[3], 0x1));

   REQUIRE(world.terrain.texture_weight_maps_dirty[1].size() == 1);
   CHECK(world.terrain.texture_weight_maps_dirty[1][0] ==
         world::dirty_rect{0, 0, 32, 32});
   REQUIRE(world.terrain.texture_weight_maps_dirty[3].size() == 1);
   CHECK(world.terrain.texture_weight_maps_dirty[3][0] ==
         world::dirty_rect{0, 0, 32, 32});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(world.terrain.texture_names[1] == "dirt");
   CHECK(world.terrain.texture_names[3] == "snow");

   CHECK(world.terrain.texture_scales[1] == 2.0f);
   CHECK(world.terrain.texture_scales[3] == 3.0f);

   CHECK(world.terrain.texture_axes[1] == world::texture_axis::yx);
   CHECK(world.terrain.texture_axes[3] == world::texture_axis::negative_yx);

   CHECK(check_map(world.terrain.texture_weight_maps[1], 0x1));
   CHECK(check_map(world.terrain.texture_weight_maps[3], 0x3));

   REQUIRE(world.terrain.texture_weight_maps_dirty[1].size() == 1);
   CHECK(world.terrain.texture_weight_maps_dirty[1][0] ==
         world::dirty_rect{0, 0, 32, 32});
   REQUIRE(world.terrain.texture_weight_maps_dirty[3].size() == 1);
   CHECK(world.terrain.texture_weight_maps_dirty[3][0] ==
         world::dirty_rect{0, 0, 32, 32});
}

}
