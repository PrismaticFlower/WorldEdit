#include "pch.h"

#include "edits/add_block.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_block box", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   const world::block_box_id id = world.blocks.next_id.boxes.aquire();

   auto edit = make_add_block({.rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                               .position = {10.0f, 10.0f, 10.0f},
                               .size = {5.0f, 5.0f, 5.0f}},
                              2, id);

   edit->apply(edit_context);

   world::blocks& blocks = world.blocks;

   REQUIRE(blocks.boxes.size() == 1);

   REQUIRE(blocks.boxes.bbox.min_x.size() == 1);
   REQUIRE(blocks.boxes.bbox.min_y.size() == 1);
   REQUIRE(blocks.boxes.bbox.min_z.size() == 1);
   REQUIRE(blocks.boxes.bbox.max_x.size() == 1);
   REQUIRE(blocks.boxes.bbox.max_y.size() == 1);
   REQUIRE(blocks.boxes.bbox.max_z.size() == 1);
   REQUIRE(blocks.boxes.hidden.size() == 1);
   REQUIRE(blocks.boxes.layer.size() == 1);
   REQUIRE(blocks.boxes.description.size() == 1);
   REQUIRE(blocks.boxes.ids.size() == 1);

   CHECK(blocks.boxes.bbox.min_x[0] == 5.0f);
   CHECK(blocks.boxes.bbox.min_y[0] == 5.0f);
   CHECK(blocks.boxes.bbox.min_z[0] == 5.0f);
   CHECK(blocks.boxes.bbox.max_x[0] == 15.0f);
   CHECK(blocks.boxes.bbox.max_y[0] == 15.0f);
   CHECK(blocks.boxes.bbox.max_z[0] == 15.0f);
   CHECK(not blocks.boxes.hidden[0]);
   CHECK(blocks.boxes.layer[0] == 2);
   CHECK(blocks.boxes.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[0].position == float3{10.0f, 10.0f, 10.0f});
   CHECK(blocks.boxes.description[0].size == float3{5.0f, 5.0f, 5.0f});
   CHECK(blocks.boxes.ids[0] == id);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{0, 1});

   edit->revert(edit_context);

   REQUIRE(blocks.boxes.size() == 0);

   REQUIRE(blocks.boxes.bbox.min_x.size() == 0);
   REQUIRE(blocks.boxes.bbox.min_y.size() == 0);
   REQUIRE(blocks.boxes.bbox.min_z.size() == 0);
   REQUIRE(blocks.boxes.bbox.max_x.size() == 0);
   REQUIRE(blocks.boxes.bbox.max_y.size() == 0);
   REQUIRE(blocks.boxes.bbox.max_z.size() == 0);
   REQUIRE(blocks.boxes.hidden.size() == 0);
   REQUIRE(blocks.boxes.layer.size() == 0);
   REQUIRE(blocks.boxes.description.size() == 0);
   REQUIRE(blocks.boxes.ids.size() == 0);

   REQUIRE(blocks.boxes.dirty.size() == 0);
}

TEST_CASE("edits add_block stairway", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   const world::block_custom_id id = world.blocks.next_id.custom.aquire();

   const world::block_description_custom stairway_desc = {
      .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
      .position = {10.0f, 10.0f, 10.0f},
      .mesh_description =
         world::block_custom_mesh_description_stairway{
            .size = {5.0f, 5.0f, 5.0f},
            .step_height = 2.5f,
            .first_step_offset = -0.1f,
         },
   };

   auto edit = make_add_block(stairway_desc, 2, id);

   const world::block_custom_mesh_description custom_mesh_desc =
      stairway_desc.mesh_description;

   edit->apply(edit_context);

   world::blocks& blocks = world.blocks;

   REQUIRE(blocks.custom.size() == 1);

   REQUIRE(blocks.custom.bbox.min_x.size() == 1);
   REQUIRE(blocks.custom.bbox.min_y.size() == 1);
   REQUIRE(blocks.custom.bbox.min_z.size() == 1);
   REQUIRE(blocks.custom.bbox.max_x.size() == 1);
   REQUIRE(blocks.custom.bbox.max_y.size() == 1);
   REQUIRE(blocks.custom.bbox.max_z.size() == 1);
   REQUIRE(blocks.custom.hidden.size() == 1);
   REQUIRE(blocks.custom.layer.size() == 1);
   REQUIRE(blocks.custom.description.size() == 1);
   REQUIRE(blocks.custom.mesh.size() == 1);
   REQUIRE(blocks.custom.ids.size() == 1);

   CHECK(blocks.custom.bbox.min_x[0] == 7.5f);
   CHECK(blocks.custom.bbox.min_y[0] == 5.1f);
   CHECK(blocks.custom.bbox.min_z[0] == 7.5f);
   CHECK(blocks.custom.bbox.max_x[0] == 12.5f);
   CHECK(blocks.custom.bbox.max_y[0] == 10.0f);
   CHECK(blocks.custom.bbox.max_z[0] == 12.5f);
   CHECK(not blocks.custom.hidden[0]);
   CHECK(blocks.custom.layer[0] == 2);
   CHECK(blocks.custom.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[0].position == float3{10.0f, 10.0f, 10.0f});
   CHECK(blocks.custom.description[0].mesh_description.stairway.size ==
         float3{5.0f, 5.0f, 5.0f});
   CHECK(blocks.custom.description[0].mesh_description.stairway.step_height == 2.5f);
   CHECK(blocks.custom.description[0].mesh_description.stairway.first_step_offset ==
         -0.1f);
   CHECK(blocks.custom.mesh[0] ==
         blocks.custom_meshes.debug_query_handle(custom_mesh_desc));
   CHECK(blocks.custom.ids[0] == id);

   REQUIRE(blocks.custom.dirty.size() == 1);
   CHECK(blocks.custom.dirty[0] == world::blocks_dirty_range{0, 1});

   CHECK(blocks.custom_meshes.debug_ref_count(custom_mesh_desc) == 1);

   edit->revert(edit_context);

   REQUIRE(blocks.custom.size() == 0);

   REQUIRE(blocks.custom.bbox.min_x.size() == 0);
   REQUIRE(blocks.custom.bbox.min_y.size() == 0);
   REQUIRE(blocks.custom.bbox.min_z.size() == 0);
   REQUIRE(blocks.custom.bbox.max_x.size() == 0);
   REQUIRE(blocks.custom.bbox.max_y.size() == 0);
   REQUIRE(blocks.custom.bbox.max_z.size() == 0);
   REQUIRE(blocks.custom.hidden.size() == 0);
   REQUIRE(blocks.custom.layer.size() == 0);
   REQUIRE(blocks.custom.description.size() == 0);
   REQUIRE(blocks.custom.mesh.size() == 0);
   REQUIRE(blocks.custom.ids.size() == 0);

   REQUIRE(blocks.custom.dirty.size() == 0);

   CHECK(blocks.custom_meshes.debug_ref_count(custom_mesh_desc) == 0);
}

}
