#include "pch.h"

#include "edits/set_block.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits set_block_box_metrics", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.boxes.bbox.min_x.push_back(5.0f);
   blocks.boxes.bbox.min_y.push_back(5.0f);
   blocks.boxes.bbox.min_z.push_back(5.0f);
   blocks.boxes.bbox.max_x.push_back(15.0f);
   blocks.boxes.bbox.max_y.push_back(15.0f);
   blocks.boxes.bbox.max_z.push_back(15.0f);
   blocks.boxes.hidden.push_back(false);
   blocks.boxes.layer.push_back(0);
   blocks.boxes.description.push_back({.rotation = quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                       .position = float3{10.0f, 10.0f, 10.0f},
                                       .size = float3{5.0f, 5.0f, 5.0f}});
   blocks.boxes.ids.push_back(world::block_box_id{});

   auto edit = make_set_block_box_metrics(0, quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                          float3{2.0f, 2.0f, 2.0f},
                                          float3{3.0f, 3.0f, 3.0f});

   edit->apply(edit_context);

   CHECK(blocks.boxes.bbox.min_x[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_y[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_z[0] == -1.0f);
   CHECK(blocks.boxes.bbox.max_x[0] == 5.0f);
   CHECK(blocks.boxes.bbox.max_y[0] == 5.0f);
   CHECK(blocks.boxes.bbox.max_z[0] == 5.0f);
   CHECK(not blocks.boxes.hidden[0]);
   CHECK(blocks.boxes.layer[0] == 0);
   CHECK(blocks.boxes.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].position == float3{2.0f, 2.0f, 2.0f});
   CHECK(blocks.boxes.description[0].size == float3{3.0f, 3.0f, 3.0f});

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});

   blocks.boxes.dirty.clear();

   edit->revert(edit_context);

   CHECK(blocks.boxes.bbox.min_x[0] == 5.0f);
   CHECK(blocks.boxes.bbox.min_y[0] == 5.0f);
   CHECK(blocks.boxes.bbox.min_z[0] == 5.0f);
   CHECK(blocks.boxes.bbox.max_x[0] == 15.0f);
   CHECK(blocks.boxes.bbox.max_y[0] == 15.0f);
   CHECK(blocks.boxes.bbox.max_z[0] == 15.0f);
   CHECK(not blocks.boxes.hidden[0]);
   CHECK(blocks.boxes.layer[0] == 0);
   CHECK(blocks.boxes.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].position == float3{10.0f, 10.0f, 10.0f});
   CHECK(blocks.boxes.description[0].size == float3{5.0f, 5.0f, 5.0f});

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});
}

TEST_CASE("edits set_block_box_metrics coalesce", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.boxes.bbox.min_x.push_back(5.0f);
   blocks.boxes.bbox.min_y.push_back(5.0f);
   blocks.boxes.bbox.min_z.push_back(5.0f);
   blocks.boxes.bbox.max_x.push_back(15.0f);
   blocks.boxes.bbox.max_y.push_back(15.0f);
   blocks.boxes.bbox.max_z.push_back(15.0f);
   blocks.boxes.hidden.push_back(false);
   blocks.boxes.layer.push_back(0);
   blocks.boxes.description.push_back({.rotation = quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                       .position = float3{10.0f, 10.0f, 10.0f},
                                       .size = float3{5.0f, 5.0f, 5.0f}});
   blocks.boxes.ids.push_back(world::block_box_id{});

   auto edit = make_set_block_box_metrics(0, quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                          float3{4.0f, 2.0f, 2.0f},
                                          float3{3.0f, 7.0f, 3.0f});
   auto other_edit =
      make_set_block_box_metrics(0, quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                 float3{2.0f, 2.0f, 2.0f}, float3{3.0f, 3.0f, 3.0f});

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(blocks.boxes.bbox.min_x[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_y[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_z[0] == -1.0f);
   CHECK(blocks.boxes.bbox.max_x[0] == 5.0f);
   CHECK(blocks.boxes.bbox.max_y[0] == 5.0f);
   CHECK(blocks.boxes.bbox.max_z[0] == 5.0f);
   CHECK(not blocks.boxes.hidden[0]);
   CHECK(blocks.boxes.layer[0] == 0);
   CHECK(blocks.boxes.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].position == float3{2.0f, 2.0f, 2.0f});
   CHECK(blocks.boxes.description[0].size == float3{3.0f, 3.0f, 3.0f});

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});

   blocks.boxes.dirty.clear();

   edit->revert(edit_context);

   CHECK(blocks.boxes.bbox.min_x[0] == 5.0f);
   CHECK(blocks.boxes.bbox.min_y[0] == 5.0f);
   CHECK(blocks.boxes.bbox.min_z[0] == 5.0f);
   CHECK(blocks.boxes.bbox.max_x[0] == 15.0f);
   CHECK(blocks.boxes.bbox.max_y[0] == 15.0f);
   CHECK(blocks.boxes.bbox.max_z[0] == 15.0f);
   CHECK(not blocks.boxes.hidden[0]);
   CHECK(blocks.boxes.layer[0] == 0);
   CHECK(blocks.boxes.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].position == float3{10.0f, 10.0f, 10.0f});
   CHECK(blocks.boxes.description[0].size == float3{5.0f, 5.0f, 5.0f});

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});
}

TEST_CASE("edits set_block_box_metrics no coalesce", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_block_box_metrics(0, quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                          float3{4.0f, 2.0f, 2.0f},
                                          float3{3.0f, 7.0f, 3.0f});
   auto other_edit =
      make_set_block_box_metrics(1, quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                 float3{2.0f, 2.0f, 2.0f}, float3{3.0f, 3.0f, 3.0f});

   REQUIRE(not edit->is_coalescable(*other_edit));
}

TEST_CASE("edits set_block_quad_metrics", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.quads.bbox.min_x.push_back(0.0f);
   blocks.quads.bbox.min_y.push_back(0.0f);
   blocks.quads.bbox.min_z.push_back(0.0f);
   blocks.quads.bbox.max_x.push_back(15.0f);
   blocks.quads.bbox.max_y.push_back(15.0f);
   blocks.quads.bbox.max_z.push_back(15.0f);
   blocks.quads.hidden.push_back(false);
   blocks.quads.layer.push_back(0);
   blocks.quads.description.push_back({
      .vertices = {{
         {15.0f, 15.0f, 15.0f},
         {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, 0.0f},
         {5.0f, 5.0f, 5.0f},
      }},
   });
   blocks.quads.ids.push_back(world::block_quad_id{});

   auto edit = make_set_block_quad_metrics(0, {
                                                 float3{5.0f, 5.0f, 5.0f},
                                                 float3{0.0f, 0.0f, 0.0f},
                                                 float3{0.0f, 0.0f, 0.0f},
                                                 float3{-1.0f, -1.0f, -1.0f},
                                              });

   edit->apply(edit_context);

   CHECK(blocks.quads.bbox.min_x[0] == -1.0f);
   CHECK(blocks.quads.bbox.min_y[0] == -1.0f);
   CHECK(blocks.quads.bbox.min_z[0] == -1.0f);
   CHECK(blocks.quads.bbox.max_x[0] == 5.0f);
   CHECK(blocks.quads.bbox.max_y[0] == 5.0f);
   CHECK(blocks.quads.bbox.max_z[0] == 5.0f);
   CHECK(not blocks.quads.hidden[0]);
   CHECK(blocks.quads.layer[0] == 0);
   CHECK(blocks.quads.description[0].vertices[0] == float3{5.0f, 5.0f, 5.0f});
   CHECK(blocks.quads.description[0].vertices[1] == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.quads.description[0].vertices[2] == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.quads.description[0].vertices[3] == float3{-1.0f, -1.0f, -1.0f});

   REQUIRE(blocks.quads.dirty.size() == 1);
   CHECK(blocks.quads.dirty[0] == world::blocks_dirty_range{0, 1});

   blocks.quads.dirty.clear();

   edit->revert(edit_context);

   CHECK(blocks.quads.bbox.min_x[0] == 0.0f);
   CHECK(blocks.quads.bbox.min_y[0] == 0.0f);
   CHECK(blocks.quads.bbox.min_z[0] == 0.0f);
   CHECK(blocks.quads.bbox.max_x[0] == 15.0f);
   CHECK(blocks.quads.bbox.max_y[0] == 15.0f);
   CHECK(blocks.quads.bbox.max_z[0] == 15.0f);
   CHECK(not blocks.quads.hidden[0]);
   CHECK(blocks.quads.layer[0] == 0);
   CHECK(blocks.quads.description[0].vertices[0] == float3{15.0f, 15.0f, 15.0f});
   CHECK(blocks.quads.description[0].vertices[1] == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.quads.description[0].vertices[2] == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.quads.description[0].vertices[3] == float3{5.0f, 5.0f, 5.0f});

   REQUIRE(blocks.quads.dirty.size() == 1);
   CHECK(blocks.quads.dirty[0] == world::blocks_dirty_range{0, 1});
}

TEST_CASE("edits set_block_quad_metrics coalesce", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.quads.bbox.min_x.push_back(0.0f);
   blocks.quads.bbox.min_y.push_back(0.0f);
   blocks.quads.bbox.min_z.push_back(0.0f);
   blocks.quads.bbox.max_x.push_back(15.0f);
   blocks.quads.bbox.max_y.push_back(15.0f);
   blocks.quads.bbox.max_z.push_back(15.0f);
   blocks.quads.hidden.push_back(false);
   blocks.quads.layer.push_back(0);
   blocks.quads.description.push_back({
      .vertices = {{
         {15.0f, 15.0f, 15.0f},
         {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, 0.0f},
         {5.0f, 5.0f, 5.0f},
      }},
   });
   blocks.quads.ids.push_back(world::block_quad_id{});

   auto edit = make_set_block_quad_metrics(0, {
                                                 float3{7.0f, 7.0f, 7.0f},
                                                 float3{0.0f, 0.0f, 0.0f},
                                                 float3{0.0f, 0.0f, 0.0f},
                                                 float3{-1.0f, -1.0f, -1.0f},
                                              });
   auto other_edit = make_set_block_quad_metrics(0, {
                                                       float3{5.0f, 5.0f, 5.0f},
                                                       float3{0.0f, 0.0f, 0.0f},
                                                       float3{0.0f, 0.0f, 0.0f},
                                                       float3{-1.0f, -1.0f, -1.0f},
                                                    });

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(blocks.quads.bbox.min_x[0] == -1.0f);
   CHECK(blocks.quads.bbox.min_y[0] == -1.0f);
   CHECK(blocks.quads.bbox.min_z[0] == -1.0f);
   CHECK(blocks.quads.bbox.max_x[0] == 5.0f);
   CHECK(blocks.quads.bbox.max_y[0] == 5.0f);
   CHECK(blocks.quads.bbox.max_z[0] == 5.0f);
   CHECK(not blocks.quads.hidden[0]);
   CHECK(blocks.quads.layer[0] == 0);
   CHECK(blocks.quads.description[0].vertices[0] == float3{5.0f, 5.0f, 5.0f});
   CHECK(blocks.quads.description[0].vertices[1] == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.quads.description[0].vertices[2] == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.quads.description[0].vertices[3] == float3{-1.0f, -1.0f, -1.0f});

   REQUIRE(blocks.quads.dirty.size() == 1);
   CHECK(blocks.quads.dirty[0] == world::blocks_dirty_range{0, 1});

   blocks.quads.dirty.clear();

   edit->revert(edit_context);

   CHECK(blocks.quads.bbox.min_x[0] == 0.0f);
   CHECK(blocks.quads.bbox.min_y[0] == 0.0f);
   CHECK(blocks.quads.bbox.min_z[0] == 0.0f);
   CHECK(blocks.quads.bbox.max_x[0] == 15.0f);
   CHECK(blocks.quads.bbox.max_y[0] == 15.0f);
   CHECK(blocks.quads.bbox.max_z[0] == 15.0f);
   CHECK(not blocks.quads.hidden[0]);
   CHECK(blocks.quads.layer[0] == 0);
   CHECK(blocks.quads.description[0].vertices[0] == float3{15.0f, 15.0f, 15.0f});
   CHECK(blocks.quads.description[0].vertices[1] == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.quads.description[0].vertices[2] == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.quads.description[0].vertices[3] == float3{5.0f, 5.0f, 5.0f});

   REQUIRE(blocks.quads.dirty.size() == 1);
   CHECK(blocks.quads.dirty[0] == world::blocks_dirty_range{0, 1});
}

TEST_CASE("edits set_block_quad_metrics no coalesce", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_block_quad_metrics(0, {
                                                 float3{5.0f, 5.0f, 5.0f},
                                                 float3{0.0f, 0.0f, 0.0f},
                                                 float3{0.0f, 0.0f, 0.0f},
                                                 float3{-1.0f, -1.0f, -1.0f},
                                              });
   auto other_edit = make_set_block_quad_metrics(1, {
                                                       float3{5.0f, 5.0f, 5.0f},
                                                       float3{0.0f, 0.0f, 0.0f},
                                                       float3{0.0f, 0.0f, 0.0f},
                                                       float3{-1.0f, -1.0f, -1.0f},
                                                    });

   REQUIRE(not edit->is_coalescable(*other_edit));
}

TEST_CASE("edits set_block_surface", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.boxes.bbox.min_x.push_back(5.0f);
   blocks.boxes.bbox.min_y.push_back(5.0f);
   blocks.boxes.bbox.min_z.push_back(5.0f);
   blocks.boxes.bbox.max_x.push_back(15.0f);
   blocks.boxes.bbox.max_y.push_back(15.0f);
   blocks.boxes.bbox.max_z.push_back(15.0f);
   blocks.boxes.hidden.push_back(false);
   blocks.boxes.layer.push_back(0);
   blocks.boxes.description.push_back({.rotation = quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                       .position = float3{10.0f, 10.0f, 10.0f},
                                       .size = float3{5.0f, 5.0f, 5.0f}});
   blocks.boxes.ids.push_back(world::block_box_id{});

   auto edit =
      make_set_block_surface(&blocks.boxes.description[0].surface_texture_rotation[1],
                             world::block_texture_rotation::d180, 0,
                             &blocks.boxes.dirty);

   edit->apply(edit_context);

   CHECK(blocks.boxes.description[0].surface_texture_rotation[1] ==
         world::block_texture_rotation::d180);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});

   blocks.boxes.dirty.clear();

   edit->revert(edit_context);

   CHECK(blocks.boxes.description[0].surface_texture_rotation[1] ==
         world::block_texture_rotation::d0);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});
}

TEST_CASE("edits set_block_surface coalesce", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.boxes.bbox.min_x.push_back(5.0f);
   blocks.boxes.bbox.min_y.push_back(5.0f);
   blocks.boxes.bbox.min_z.push_back(5.0f);
   blocks.boxes.bbox.max_x.push_back(15.0f);
   blocks.boxes.bbox.max_y.push_back(15.0f);
   blocks.boxes.bbox.max_z.push_back(15.0f);
   blocks.boxes.hidden.push_back(false);
   blocks.boxes.layer.push_back(0);
   blocks.boxes.description.push_back({.rotation = quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                       .position = float3{10.0f, 10.0f, 10.0f},
                                       .size = float3{5.0f, 5.0f, 5.0f}});
   blocks.boxes.ids.push_back(world::block_box_id{});

   auto edit =
      make_set_block_surface(&blocks.boxes.description[0].surface_texture_rotation[1],
                             world::block_texture_rotation::d180, 0,
                             &blocks.boxes.dirty);
   auto other_edit =
      make_set_block_surface(&blocks.boxes.description[0].surface_texture_rotation[1],
                             world::block_texture_rotation::d90, 0,
                             &blocks.boxes.dirty);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(blocks.boxes.description[0].surface_texture_rotation[1] ==
         world::block_texture_rotation::d90);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});

   blocks.boxes.dirty.clear();

   edit->revert(edit_context);

   CHECK(blocks.boxes.description[0].surface_texture_rotation[1] ==
         world::block_texture_rotation::d0);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});
}

TEST_CASE("edits set_block_box_surface no coalesce", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   blocks.boxes.description.push_back({});
   blocks.boxes.description.push_back({});

   auto edit =
      make_set_block_surface(&blocks.boxes.description[0].surface_texture_rotation[1],
                             world::block_texture_rotation::d180, 0,
                             &blocks.boxes.dirty);
   auto other_edit =
      make_set_block_surface(&blocks.boxes.description[1].surface_texture_rotation[1],
                             world::block_texture_rotation::d90, 1,
                             &blocks.boxes.dirty);

   REQUIRE(not edit->is_coalescable(*other_edit));
}

}
