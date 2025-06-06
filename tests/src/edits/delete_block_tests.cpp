#include "pch.h"

#include "edits/add_block.hpp"
#include "edits/delete_block.hpp"
#include "world/world.hpp"

namespace we::edits::tests {

TEST_CASE("edits delete_block (box)", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   const world::block_box_id id0 = blocks.next_id.boxes.aquire();
   const world::block_box_id id1 = blocks.next_id.boxes.aquire();
   const world::block_box_id id2 = blocks.next_id.boxes.aquire();
   const world::block_box_id id3 = blocks.next_id.boxes.aquire();

   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{1.0f, 1.0f, 1.0f}},
                  1, id0)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{2.0f, 2.0f, 2.0f}},
                  1, id1)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{3.0f, 3.0f, 3.0f}},
                  0, id2)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{4.0f, 4.0f, 4.0f}},
                  0, id3)
      ->apply(edit_context);

   blocks.boxes.hidden[1] = true;
   blocks.boxes.dirty.clear();

   auto edit = make_delete_block(world::block_type::box, 1);

   edit->apply(edit_context);

   REQUIRE(blocks.boxes.size() == 3);
   CHECK(blocks.boxes.is_balanced());

   CHECK(blocks.boxes.bbox.min_x[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_y[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_z[0] == -1.0f);
   CHECK(blocks.boxes.bbox.max_x[0] == 1.0f);
   CHECK(blocks.boxes.bbox.max_y[0] == 1.0f);
   CHECK(blocks.boxes.bbox.max_z[0] == 1.0f);
   CHECK(not blocks.boxes.hidden[0]);
   CHECK(blocks.boxes.layer[0] == 1);
   CHECK(blocks.boxes.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].size == float3{1.0f, 1.0f, 1.0f});
   CHECK(blocks.boxes.ids[0] == id0);

   CHECK(blocks.boxes.bbox.min_x[1] == -3.0f);
   CHECK(blocks.boxes.bbox.min_y[1] == -3.0f);
   CHECK(blocks.boxes.bbox.min_z[1] == -3.0f);
   CHECK(blocks.boxes.bbox.max_x[1] == 3.0f);
   CHECK(blocks.boxes.bbox.max_y[1] == 3.0f);
   CHECK(blocks.boxes.bbox.max_z[1] == 3.0f);
   CHECK(not blocks.boxes.hidden[1]);
   CHECK(blocks.boxes.layer[1] == 0);
   CHECK(blocks.boxes.description[1].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[1].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[1].size == float3{3.0f, 3.0f, 3.0f});
   CHECK(blocks.boxes.ids[1] == id2);

   CHECK(blocks.boxes.bbox.min_x[2] == -4.0f);
   CHECK(blocks.boxes.bbox.min_y[2] == -4.0f);
   CHECK(blocks.boxes.bbox.min_z[2] == -4.0f);
   CHECK(blocks.boxes.bbox.max_x[2] == 4.0f);
   CHECK(blocks.boxes.bbox.max_y[2] == 4.0f);
   CHECK(blocks.boxes.bbox.max_z[2] == 4.0f);
   CHECK(not blocks.boxes.hidden[2]);
   CHECK(blocks.boxes.layer[2] == 0);
   CHECK(blocks.boxes.description[2].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[2].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[2].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(blocks.boxes.ids[2] == id3);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{1, 3});

   blocks.boxes.dirty.clear();

   edit->revert(edit_context);

   REQUIRE(blocks.boxes.size() == 4);
   CHECK(blocks.boxes.is_balanced());

   CHECK(blocks.boxes.bbox.min_x[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_y[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_z[0] == -1.0f);
   CHECK(blocks.boxes.bbox.max_x[0] == 1.0f);
   CHECK(blocks.boxes.bbox.max_y[0] == 1.0f);
   CHECK(blocks.boxes.bbox.max_z[0] == 1.0f);
   CHECK(not blocks.boxes.hidden[0]);
   CHECK(blocks.boxes.layer[0] == 1);
   CHECK(blocks.boxes.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].size == float3{1.0f, 1.0f, 1.0f});
   CHECK(blocks.boxes.ids[0] == id0);

   CHECK(blocks.boxes.bbox.min_x[1] == -2.0f);
   CHECK(blocks.boxes.bbox.min_y[1] == -2.0f);
   CHECK(blocks.boxes.bbox.min_z[1] == -2.0f);
   CHECK(blocks.boxes.bbox.max_x[1] == 2.0f);
   CHECK(blocks.boxes.bbox.max_y[1] == 2.0f);
   CHECK(blocks.boxes.bbox.max_z[1] == 2.0f);
   CHECK(blocks.boxes.hidden[1]);
   CHECK(blocks.boxes.layer[1] == 1);
   CHECK(blocks.boxes.description[1].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[1].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[1].size == float3{2.0f, 2.0f, 2.0f});
   CHECK(blocks.boxes.ids[1] == id1);

   CHECK(blocks.boxes.bbox.min_x[2] == -3.0f);
   CHECK(blocks.boxes.bbox.min_y[2] == -3.0f);
   CHECK(blocks.boxes.bbox.min_z[2] == -3.0f);
   CHECK(blocks.boxes.bbox.max_x[2] == 3.0f);
   CHECK(blocks.boxes.bbox.max_y[2] == 3.0f);
   CHECK(blocks.boxes.bbox.max_z[2] == 3.0f);
   CHECK(not blocks.boxes.hidden[2]);
   CHECK(blocks.boxes.layer[2] == 0);
   CHECK(blocks.boxes.description[2].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[2].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[2].size == float3{3.0f, 3.0f, 3.0f});
   CHECK(blocks.boxes.ids[2] == id2);

   CHECK(blocks.boxes.bbox.min_x[3] == -4.0f);
   CHECK(blocks.boxes.bbox.min_y[3] == -4.0f);
   CHECK(blocks.boxes.bbox.min_z[3] == -4.0f);
   CHECK(blocks.boxes.bbox.max_x[3] == 4.0f);
   CHECK(blocks.boxes.bbox.max_y[3] == 4.0f);
   CHECK(blocks.boxes.bbox.max_z[3] == 4.0f);
   CHECK(not blocks.boxes.hidden[3]);
   CHECK(blocks.boxes.layer[3] == 0);
   CHECK(blocks.boxes.description[3].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[3].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[3].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(blocks.boxes.ids[3] == id3);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{1, 4});
}

TEST_CASE("edits delete_block (stairway)", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::blocks& blocks = world.blocks;

   const world::block_stairway_id id0 = blocks.next_id.stairways.aquire();
   const world::block_stairway_id id1 = blocks.next_id.stairways.aquire();
   const world::block_stairway_id id2 = blocks.next_id.stairways.aquire();
   const world::block_stairway_id id3 = blocks.next_id.stairways.aquire();
   const world::block_description_stairway stair_desc0 =
      {.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
       .position = float3{0.0f, 0.0f, 0.0f},
       .size = float3{1.0f, 1.0f, 1.0f}};
   const world::block_description_stairway stair_desc1 =
      {.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
       .position = float3{0.0f, 0.0f, 0.0f},
       .size = float3{2.0f, 2.0f, 2.0f}};
   const world::block_description_stairway stair_desc2 =
      {.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
       .position = float3{0.0f, 0.0f, 0.0f},
       .size = float3{3.0f, 3.0f, 3.0f}};
   const world::block_description_stairway stair_desc3 =
      {.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
       .position = float3{0.0f, 0.0f, 0.0f},
       .size = float3{4.0f, 4.0f, 4.0f}};

   make_add_block(stair_desc0, 1, id0)->apply(edit_context);
   make_add_block(stair_desc1, 1, id1)->apply(edit_context);
   make_add_block(stair_desc2, 0, id2)->apply(edit_context);
   make_add_block(stair_desc3, 0, id3)->apply(edit_context);

   blocks.stairways.hidden[1] = true;
   blocks.stairways.dirty.clear();

   auto edit = make_delete_block(world::block_type::stairway, 1);

   edit->apply(edit_context);

   REQUIRE(blocks.stairways.size() == 3);
   CHECK(blocks.stairways.is_balanced());

   CHECK(blocks.stairways.bbox.min_x[0] == -0.5f);
   CHECK(blocks.stairways.bbox.min_y[0] == 0.0f);
   CHECK(blocks.stairways.bbox.min_z[0] == -0.5f);
   CHECK(blocks.stairways.bbox.max_x[0] == 0.5f);
   CHECK(blocks.stairways.bbox.max_y[0] == 1.0f);
   CHECK(blocks.stairways.bbox.max_z[0] == 0.5f);
   CHECK(not blocks.stairways.hidden[0]);
   CHECK(blocks.stairways.layer[0] == 1);
   CHECK(blocks.stairways.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[0].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[0].size == float3{1.0f, 1.0f, 1.0f});
   CHECK(blocks.stairways.mesh[0] ==
         blocks.custom_meshes.debug_query_handle(stair_desc0.custom_mesh_desc()));
   CHECK(blocks.stairways.ids[0] == id0);

   CHECK(blocks.stairways.bbox.min_x[1] == -1.5f);
   CHECK(blocks.stairways.bbox.min_y[1] == 0.0f);
   CHECK(blocks.stairways.bbox.min_z[1] == -1.5f);
   CHECK(blocks.stairways.bbox.max_x[1] == 1.5f);
   CHECK(blocks.stairways.bbox.max_y[1] == 3.0f);
   CHECK(blocks.stairways.bbox.max_z[1] == 1.5f);
   CHECK(not blocks.stairways.hidden[1]);
   CHECK(blocks.stairways.layer[1] == 0);
   CHECK(blocks.stairways.description[1].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[1].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[1].size == float3{3.0f, 3.0f, 3.0f});
   CHECK(blocks.stairways.mesh[1] ==
         blocks.custom_meshes.debug_query_handle(stair_desc2.custom_mesh_desc()));
   CHECK(blocks.stairways.ids[1] == id2);

   CHECK(blocks.stairways.bbox.min_x[2] == -2.0f);
   CHECK(blocks.stairways.bbox.min_y[2] == 0.0f);
   CHECK(blocks.stairways.bbox.min_z[2] == -2.0f);
   CHECK(blocks.stairways.bbox.max_x[2] == 2.0f);
   CHECK(blocks.stairways.bbox.max_y[2] == 4.0f);
   CHECK(blocks.stairways.bbox.max_z[2] == 2.0f);
   CHECK(not blocks.stairways.hidden[2]);
   CHECK(blocks.stairways.layer[2] == 0);
   CHECK(blocks.stairways.description[2].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[2].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[2].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(blocks.stairways.mesh[2] ==
         blocks.custom_meshes.debug_query_handle(stair_desc3.custom_mesh_desc()));
   CHECK(blocks.stairways.ids[2] == id3);

   REQUIRE(blocks.stairways.dirty.size() == 1);
   CHECK(blocks.stairways.dirty[0] == world::blocks_dirty_range{1, 3});

   CHECK(blocks.custom_meshes.debug_ref_count(stair_desc0.custom_mesh_desc()) == 1);
   CHECK(blocks.custom_meshes.debug_ref_count(stair_desc1.custom_mesh_desc()) == 0);
   CHECK(blocks.custom_meshes.debug_ref_count(stair_desc2.custom_mesh_desc()) == 1);
   CHECK(blocks.custom_meshes.debug_ref_count(stair_desc3.custom_mesh_desc()) == 1);

   blocks.stairways.dirty.clear();

   edit->revert(edit_context);

   REQUIRE(blocks.stairways.size() == 4);
   CHECK(blocks.stairways.is_balanced());

   CHECK(blocks.stairways.bbox.min_x[0] == -0.5f);
   CHECK(blocks.stairways.bbox.min_y[0] == 0.0f);
   CHECK(blocks.stairways.bbox.min_z[0] == -0.5f);
   CHECK(blocks.stairways.bbox.max_x[0] == 0.5f);
   CHECK(blocks.stairways.bbox.max_y[0] == 1.0f);
   CHECK(blocks.stairways.bbox.max_z[0] == 0.5f);
   CHECK(not blocks.stairways.hidden[0]);
   CHECK(blocks.stairways.layer[0] == 1);
   CHECK(blocks.stairways.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[0].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[0].size == float3{1.0f, 1.0f, 1.0f});
   CHECK(blocks.stairways.mesh[0] ==
         blocks.custom_meshes.debug_query_handle(stair_desc0.custom_mesh_desc()));
   CHECK(blocks.stairways.ids[0] == id0);

   CHECK(blocks.stairways.bbox.min_x[1] == -1.0f);
   CHECK(blocks.stairways.bbox.min_y[1] == 0.0f);
   CHECK(blocks.stairways.bbox.min_z[1] == -1.0f);
   CHECK(blocks.stairways.bbox.max_x[1] == 1.0f);
   CHECK(blocks.stairways.bbox.max_y[1] == 2.0f);
   CHECK(blocks.stairways.bbox.max_z[1] == 1.0f);
   CHECK(blocks.stairways.hidden[1]);
   CHECK(blocks.stairways.layer[1] == 1);
   CHECK(blocks.stairways.description[1].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[1].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[1].size == float3{2.0f, 2.0f, 2.0f});
   CHECK(blocks.stairways.mesh[1] ==
         blocks.custom_meshes.debug_query_handle(stair_desc1.custom_mesh_desc()));
   CHECK(blocks.stairways.ids[1] == id1);

   CHECK(blocks.stairways.bbox.min_x[2] == -1.5f);
   CHECK(blocks.stairways.bbox.min_y[2] == 0.0f);
   CHECK(blocks.stairways.bbox.min_z[2] == -1.5f);
   CHECK(blocks.stairways.bbox.max_x[2] == 1.5f);
   CHECK(blocks.stairways.bbox.max_y[2] == 3.0f);
   CHECK(blocks.stairways.bbox.max_z[2] == 1.5f);
   CHECK(not blocks.stairways.hidden[2]);
   CHECK(blocks.stairways.layer[2] == 0);
   CHECK(blocks.stairways.description[2].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[2].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[2].size == float3{3.0f, 3.0f, 3.0f});
   CHECK(blocks.stairways.mesh[2] ==
         blocks.custom_meshes.debug_query_handle(stair_desc2.custom_mesh_desc()));
   CHECK(blocks.stairways.ids[2] == id2);

   CHECK(blocks.stairways.bbox.min_x[3] == -2.0f);
   CHECK(blocks.stairways.bbox.min_y[3] == 0.0f);
   CHECK(blocks.stairways.bbox.min_z[3] == -2.0f);
   CHECK(blocks.stairways.bbox.max_x[3] == 2.0f);
   CHECK(blocks.stairways.bbox.max_y[3] == 4.0f);
   CHECK(blocks.stairways.bbox.max_z[3] == 2.0f);
   CHECK(not blocks.stairways.hidden[3]);
   CHECK(blocks.stairways.layer[3] == 0);
   CHECK(blocks.stairways.description[3].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[3].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.stairways.description[3].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(blocks.stairways.mesh[3] ==
         blocks.custom_meshes.debug_query_handle(stair_desc3.custom_mesh_desc()));
   CHECK(blocks.stairways.ids[3] == id3);

   REQUIRE(blocks.stairways.dirty.size() == 1);
   CHECK(blocks.stairways.dirty[0] == world::blocks_dirty_range{1, 4});

   CHECK(blocks.custom_meshes.debug_ref_count(stair_desc0.custom_mesh_desc()) == 1);
   CHECK(blocks.custom_meshes.debug_ref_count(stair_desc1.custom_mesh_desc()) == 1);
   CHECK(blocks.custom_meshes.debug_ref_count(stair_desc2.custom_mesh_desc()) == 1);
   CHECK(blocks.custom_meshes.debug_ref_count(stair_desc3.custom_mesh_desc()) == 1);
}

}