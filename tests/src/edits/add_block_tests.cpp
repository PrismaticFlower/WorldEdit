#include "pch.h"

#include "edits/add_block.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_block cube", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_add_block(
      world::block_description_cube{.rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                                    .position = {10.0f, 10.0f, 10.0f},
                                    .size = {5.0f, 5.0f, 5.0f}});

   edit->apply(edit_context);

   world::blocks& blocks = world.blocks;

   REQUIRE(blocks.cubes.size() == 1);

   REQUIRE(blocks.cubes.bbox.min_x.size() == 1);
   REQUIRE(blocks.cubes.bbox.min_y.size() == 1);
   REQUIRE(blocks.cubes.bbox.min_z.size() == 1);
   REQUIRE(blocks.cubes.bbox.max_x.size() == 1);
   REQUIRE(blocks.cubes.bbox.max_y.size() == 1);
   REQUIRE(blocks.cubes.bbox.max_z.size() == 1);
   REQUIRE(blocks.cubes.hidden.size() == 1);
   REQUIRE(blocks.cubes.description.size() == 1);

   CHECK(blocks.cubes.bbox.min_x[0] == 5.0f);
   CHECK(blocks.cubes.bbox.min_y[0] == 5.0f);
   CHECK(blocks.cubes.bbox.min_z[0] == 5.0f);
   CHECK(blocks.cubes.bbox.max_x[0] == 15.0f);
   CHECK(blocks.cubes.bbox.max_y[0] == 15.0f);
   CHECK(blocks.cubes.bbox.max_z[0] == 15.0f);
   CHECK(not blocks.cubes.hidden[0]);
   CHECK(blocks.cubes.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(blocks.cubes.description[0].position == float3{10.0f, 10.0f, 10.0f});
   CHECK(blocks.cubes.description[0].size == float3{5.0f, 5.0f, 5.0f});

   REQUIRE(blocks.cubes.dirty.size() == 1);
   CHECK(blocks.cubes.dirty[0] == world::blocks_dirty_range{0, 1});

   edit->revert(edit_context);

   REQUIRE(blocks.cubes.size() == 0);

   REQUIRE(blocks.cubes.bbox.min_x.size() == 0);
   REQUIRE(blocks.cubes.bbox.min_y.size() == 0);
   REQUIRE(blocks.cubes.bbox.min_z.size() == 0);
   REQUIRE(blocks.cubes.bbox.max_x.size() == 0);
   REQUIRE(blocks.cubes.bbox.max_y.size() == 0);
   REQUIRE(blocks.cubes.bbox.max_z.size() == 0);
   REQUIRE(blocks.cubes.hidden.size() == 0);
   REQUIRE(blocks.cubes.description.size() == 0);

   REQUIRE(blocks.cubes.dirty.size() == 0);
}
}
