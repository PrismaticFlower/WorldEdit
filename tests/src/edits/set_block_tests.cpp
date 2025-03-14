#include "pch.h"

#include "edits/set_block.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits set_block_cube_metrics", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.cubes.bbox.min_x.push_back(5.0f);
   blocks.cubes.bbox.min_y.push_back(5.0f);
   blocks.cubes.bbox.min_z.push_back(5.0f);
   blocks.cubes.bbox.max_x.push_back(15.0f);
   blocks.cubes.bbox.max_y.push_back(15.0f);
   blocks.cubes.bbox.max_z.push_back(15.0f);
   blocks.cubes.hidden.push_back(false);
   blocks.cubes.description.push_back({.rotation = quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                       .position = float3{10.0f, 10.0f, 10.0f},
                                       .size = float3{5.0f, 5.0f, 5.0f}});

   auto edit = make_set_block_cube_metrics(0, quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                           float3{2.0f, 2.0f, 2.0f},
                                           float3{3.0f, 3.0f, 3.0f});

   edit->apply(edit_context);

   CHECK(blocks.cubes.bbox.min_x[0] == -1.0f);
   CHECK(blocks.cubes.bbox.min_y[0] == -1.0f);
   CHECK(blocks.cubes.bbox.min_z[0] == -1.0f);
   CHECK(blocks.cubes.bbox.max_x[0] == 5.0f);
   CHECK(blocks.cubes.bbox.max_y[0] == 5.0f);
   CHECK(blocks.cubes.bbox.max_z[0] == 5.0f);
   CHECK(not blocks.cubes.hidden[0]);
   CHECK(blocks.cubes.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.cubes.description[0].position == float3{2.0f, 2.0f, 2.0f});
   CHECK(blocks.cubes.description[0].size == float3{3.0f, 3.0f, 3.0f});

   REQUIRE(blocks.cubes.dirty.size() == 1);
   CHECK(blocks.cubes.dirty[0] == world::blocks_dirty_range{0, 1});

   blocks.cubes.dirty.clear();

   edit->revert(edit_context);

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
}

TEST_CASE("edits set_block_cube_metrics coalesce", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.cubes.bbox.min_x.push_back(5.0f);
   blocks.cubes.bbox.min_y.push_back(5.0f);
   blocks.cubes.bbox.min_z.push_back(5.0f);
   blocks.cubes.bbox.max_x.push_back(15.0f);
   blocks.cubes.bbox.max_y.push_back(15.0f);
   blocks.cubes.bbox.max_z.push_back(15.0f);
   blocks.cubes.hidden.push_back(false);
   blocks.cubes.description.push_back({.rotation = quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                       .position = float3{10.0f, 10.0f, 10.0f},
                                       .size = float3{5.0f, 5.0f, 5.0f}});

   auto edit = make_set_block_cube_metrics(0, quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                           float3{4.0f, 2.0f, 2.0f},
                                           float3{3.0f, 7.0f, 3.0f});
   auto other_edit =
      make_set_block_cube_metrics(0, quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                  float3{2.0f, 2.0f, 2.0f}, float3{3.0f, 3.0f, 3.0f});

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(blocks.cubes.bbox.min_x[0] == -1.0f);
   CHECK(blocks.cubes.bbox.min_y[0] == -1.0f);
   CHECK(blocks.cubes.bbox.min_z[0] == -1.0f);
   CHECK(blocks.cubes.bbox.max_x[0] == 5.0f);
   CHECK(blocks.cubes.bbox.max_y[0] == 5.0f);
   CHECK(blocks.cubes.bbox.max_z[0] == 5.0f);
   CHECK(not blocks.cubes.hidden[0]);
   CHECK(blocks.cubes.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.cubes.description[0].position == float3{2.0f, 2.0f, 2.0f});
   CHECK(blocks.cubes.description[0].size == float3{3.0f, 3.0f, 3.0f});

   REQUIRE(blocks.cubes.dirty.size() == 1);
   CHECK(blocks.cubes.dirty[0] == world::blocks_dirty_range{0, 1});

   blocks.cubes.dirty.clear();

   edit->revert(edit_context);

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
}

TEST_CASE("edits set_block_cube_metrics no coalesce", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_block_cube_metrics(0, quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                           float3{4.0f, 2.0f, 2.0f},
                                           float3{3.0f, 7.0f, 3.0f});
   auto other_edit =
      make_set_block_cube_metrics(1, quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                  float3{2.0f, 2.0f, 2.0f}, float3{3.0f, 3.0f, 3.0f});

   REQUIRE(not edit->is_coalescable(*other_edit));
}

}
