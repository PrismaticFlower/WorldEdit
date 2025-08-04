#include "pch.h"

#include "edits/add_block.hpp"
#include "edits/delete_layer.hpp"
#include "world/object_class_library.hpp"
#include "world/world.hpp"

#include "null_asset_libraries.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

using world::hintnode;
using world::light;
using world::object;
using world::path;
using world::region;

const we::world::world layer_delete_test_world = {
   .name = "Test"s,

   .requirements = {{.file_type = "world",
                     .entries = {"Test", "Test_Middle", "Test_Top"}}},

   .layer_descriptions = {{.name = "[Base]"}, {.name = "Middle"}, {.name = "Top"}},
   .game_modes =
      {
         {
            .name = "conquest",
            .layers = {1},
            .requirements = {{
               .file_type = "world",
               .entries = {"Test_Middle"},
            }},
         },
      },
   .common_layers = {0, 1, 2},

   .objects = {world::entities_init,
               std::initializer_list{
                  object{.name = "Object0", .layer = 0},
                  object{.name = "Object1", .layer = 1},
                  object{.name = "Object2", .layer = 2},
                  object{.name = "Object3", .layer = 1},
                  object{.name = "Object4", .layer = 2},
               }},

   .lights = {world::entities_init,
              std::initializer_list{
                 light{.name = "Light0", .layer = 0},
                 light{.name = "Light1", .layer = 1},
                 light{.name = "Light2", .layer = 2},
              }},

   .paths = {world::entities_init,
             std::initializer_list{
                path{.name = "Path0", .layer = 0},
                path{.name = "Path1", .layer = 1},
                path{.name = "Path2", .layer = 2},
             }},

   .regions = {world::entities_init,
               std::initializer_list{
                  region{.name = "Region0", .layer = 0},
                  region{.name = "Region1", .layer = 1},
                  region{.name = "Region2", .layer = 2},
               }},

   .hintnodes = {world::entities_init,
                 std::initializer_list{
                    hintnode{.name = "Hintnode0", .layer = 0},
                    hintnode{.name = "Hintnode1", .layer = 1},
                    hintnode{.name = "Hintnode2", .layer = 2},
                 }},
};

}

TEST_CASE("edits delete_layer", "[Edits]")
{
   world::world world = layer_delete_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto action = make_delete_layer(1, world, object_class_library);

   action->apply(edit_context);

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   CHECK(world.layer_descriptions[1].name == "Top");
   REQUIRE(world.layer_descriptions.size() == 2);

   REQUIRE(world.requirements[0].entries.size() == 2);
   CHECK(world.requirements[0].entries[0] == "Test");
   CHECK(world.requirements[0].entries[1] == "Test_Top");

   CHECK(world.game_modes[0].layers.empty());
   CHECK(world.game_modes[0].requirements[0].entries.empty());

   REQUIRE(world.common_layers.size() == 2);
   CHECK(world.common_layers[0] == 0);
   CHECK(world.common_layers[1] == 1);

   REQUIRE(world.objects.size() == 3);
   CHECK(world.objects[0].name == "Object0");
   CHECK(world.objects[1].name == "Object2");
   CHECK(world.objects[2].name == "Object4");

   CHECK(world.objects[0].layer == 0);
   CHECK(world.objects[1].layer == 1);
   CHECK(world.objects[2].layer == 1);

   REQUIRE(world.lights.size() == 2);
   CHECK(world.lights[0].name == "Light0");
   CHECK(world.lights[1].name == "Light2");

   CHECK(world.lights[0].layer == 0);
   CHECK(world.lights[1].layer == 1);

   REQUIRE(world.paths.size() == 2);
   CHECK(world.paths[0].name == "Path0");
   CHECK(world.paths[1].name == "Path2");

   CHECK(world.paths[0].layer == 0);
   CHECK(world.paths[1].layer == 1);

   REQUIRE(world.regions.size() == 2);
   CHECK(world.regions[0].name == "Region0");
   CHECK(world.regions[1].name == "Region2");

   CHECK(world.regions[0].layer == 0);
   CHECK(world.regions[1].layer == 1);

   REQUIRE(world.hintnodes.size() == 2);
   CHECK(world.hintnodes[0].name == "Hintnode0");
   CHECK(world.hintnodes[1].name == "Hintnode2");

   CHECK(world.hintnodes[0].layer == 0);
   CHECK(world.hintnodes[1].layer == 1);

   REQUIRE(world.deleted_layers.size() == 1);
   CHECK(world.deleted_layers[0] == "Middle");

   action->revert(edit_context);

   CHECK(world.layer_descriptions == layer_delete_test_world.layer_descriptions);
   CHECK(world.game_modes == layer_delete_test_world.game_modes);
   CHECK(world.common_layers == layer_delete_test_world.common_layers);
   CHECK(world.objects == layer_delete_test_world.objects);
   CHECK(world.lights == layer_delete_test_world.lights);
   CHECK(world.paths == layer_delete_test_world.paths);
   CHECK(world.regions == layer_delete_test_world.regions);
   CHECK(world.hintnodes == layer_delete_test_world.hintnodes);
   CHECK(world.deleted_layers == layer_delete_test_world.deleted_layers);
}

TEST_CASE("edits delete_layer object class handle liftime", "[Edits]")
{
   world::world world = layer_delete_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   const std::array<lowercase_string, 5> object_class_names = {
      lowercase_string{"test_bldg_object0"sv},
      lowercase_string{"test_bldg_object1"sv},
      lowercase_string{"test_bldg_object2"sv},
      lowercase_string{"test_bldg_object3"sv},
      lowercase_string{"test_bldg_object4"sv},
   };

   for (std::size_t i = 0; i < object_class_names.size(); ++i) {
      world.objects[i].class_name = object_class_names[i];
      world.objects[i].class_handle =
         object_class_library.acquire(object_class_names[i]);
   }

   auto edit = make_delete_layer(1, world, object_class_library);

   edit->apply(edit_context);

   CHECK(object_class_library.debug_ref_count(object_class_names[0]) == 1);
   CHECK(object_class_library.debug_ref_count(object_class_names[1]) == 0);
   CHECK(object_class_library.debug_ref_count(object_class_names[2]) == 1);
   CHECK(object_class_library.debug_ref_count(object_class_names[3]) == 0);
   CHECK(object_class_library.debug_ref_count(object_class_names[4]) == 1);

   edit->revert(edit_context);

   CHECK(object_class_library.debug_ref_count(object_class_names[0]) == 1);
   CHECK(object_class_library.debug_ref_count(object_class_names[1]) == 1);
   CHECK(object_class_library.debug_ref_count(object_class_names[2]) == 1);
   CHECK(object_class_library.debug_ref_count(object_class_names[3]) == 1);
   CHECK(object_class_library.debug_ref_count(object_class_names[4]) == 1);
}

TEST_CASE("edits delete_layer blocks", "[Edits]")
{
   world::world world = {
      .layer_descriptions = {{.name = "[Base]"}, {.name = "Middle"}, {.name = "Top"}},
   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   world::blocks& blocks = world.blocks;

   const world::block_box_id id0 = blocks.next_id.boxes.aquire();
   const world::block_box_id id1 = blocks.next_id.boxes.aquire();
   const world::block_box_id id2 = blocks.next_id.boxes.aquire();
   const world::block_box_id id3 = blocks.next_id.boxes.aquire();
   const world::block_box_id id4 = blocks.next_id.boxes.aquire();

   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{1.0f, 1.0f, 1.0f}},
                  2, id0)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{2.0f, 2.0f, 2.0f}},
                  1, id1)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{3.0f, 3.0f, 3.0f}},
                  2, id2)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{4.0f, 4.0f, 4.0f}},
                  1, id3)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .size = float3{5.0f, 5.0f, 5.0f}},
                  2, id4)
      ->apply(edit_context);

   world.blocks.untracked_clear_dirty_ranges();

   auto edit = make_delete_layer(1, world, object_class_library);

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
   CHECK(blocks.boxes.layer[1] == 1);
   CHECK(blocks.boxes.description[1].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[1].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[1].size == float3{3.0f, 3.0f, 3.0f});
   CHECK(blocks.boxes.ids[1] == id2);

   CHECK(blocks.boxes.bbox.min_x[2] == -5.0f);
   CHECK(blocks.boxes.bbox.min_y[2] == -5.0f);
   CHECK(blocks.boxes.bbox.min_z[2] == -5.0f);
   CHECK(blocks.boxes.bbox.max_x[2] == 5.0f);
   CHECK(blocks.boxes.bbox.max_y[2] == 5.0f);
   CHECK(blocks.boxes.bbox.max_z[2] == 5.0f);
   CHECK(not blocks.boxes.hidden[2]);
   CHECK(blocks.boxes.layer[2] == 1);
   CHECK(blocks.boxes.description[2].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[2].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[2].size == float3{5.0f, 5.0f, 5.0f});
   CHECK(blocks.boxes.ids[2] == id4);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{1, 3});

   world.blocks.untracked_clear_dirty_ranges();

   edit->revert(edit_context);

   REQUIRE(blocks.boxes.size() == 5);
   CHECK(blocks.boxes.is_balanced());

   CHECK(blocks.boxes.bbox.min_x[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_y[0] == -1.0f);
   CHECK(blocks.boxes.bbox.min_z[0] == -1.0f);
   CHECK(blocks.boxes.bbox.max_x[0] == 1.0f);
   CHECK(blocks.boxes.bbox.max_y[0] == 1.0f);
   CHECK(blocks.boxes.bbox.max_z[0] == 1.0f);
   CHECK(not blocks.boxes.hidden[0]);
   CHECK(blocks.boxes.layer[0] == 2);
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
   CHECK(not blocks.boxes.hidden[1]);
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
   CHECK(blocks.boxes.layer[2] == 2);
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
   CHECK(blocks.boxes.layer[3] == 1);
   CHECK(blocks.boxes.description[3].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[3].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[3].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(blocks.boxes.ids[3] == id3);

   CHECK(blocks.boxes.bbox.min_x[4] == -5.0f);
   CHECK(blocks.boxes.bbox.min_y[4] == -5.0f);
   CHECK(blocks.boxes.bbox.min_z[4] == -5.0f);
   CHECK(blocks.boxes.bbox.max_x[4] == 5.0f);
   CHECK(blocks.boxes.bbox.max_y[4] == 5.0f);
   CHECK(blocks.boxes.bbox.max_z[4] == 5.0f);
   CHECK(not blocks.boxes.hidden[4]);
   CHECK(blocks.boxes.layer[4] == 2);
   CHECK(blocks.boxes.description[4].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[4].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.boxes.description[4].size == float3{5.0f, 5.0f, 5.0f});
   CHECK(blocks.boxes.ids[4] == id4);

   REQUIRE(blocks.boxes.dirty.size() == 1);
   CHECK(blocks.boxes.dirty[0] == world::blocks_dirty_range{1, 5});
}

TEST_CASE("edits delete_layer blocks custom", "[Edits]")
{
   world::world world = {
      .layer_descriptions = {{.name = "[Base]"}, {.name = "Middle"}, {.name = "Top"}},
   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   world::blocks& blocks = world.blocks;

   const world::block_custom_mesh_description mesh_desc0 =
      world::block_custom_mesh_description_ring{};
   const world::block_custom_mesh_description mesh_desc1 =
      world::block_custom_mesh_description_beveled_box{};

   const world::block_custom_id id0 = blocks.next_id.custom.aquire();
   const world::block_custom_id id1 = blocks.next_id.custom.aquire();
   const world::block_custom_id id2 = blocks.next_id.custom.aquire();
   const world::block_custom_id id3 = blocks.next_id.custom.aquire();
   const world::block_custom_id id4 = blocks.next_id.custom.aquire();

   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .mesh_description = mesh_desc0},
                  2, id0)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .mesh_description = mesh_desc0},
                  1, id1)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .mesh_description = mesh_desc1},
                  2, id2)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .mesh_description = mesh_desc1},
                  1, id3)
      ->apply(edit_context);
   make_add_block({.rotation = quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                   .position = float3{0.0f, 0.0f, 0.0f},
                   .mesh_description = mesh_desc1},
                  2, id4)
      ->apply(edit_context);

   world.blocks.untracked_clear_dirty_ranges();

   auto edit = make_delete_layer(1, world, object_class_library);

   edit->apply(edit_context);

   REQUIRE(blocks.custom.size() == 3);
   CHECK(blocks.custom.is_balanced());

   CHECK(blocks.custom.bbox.min_x[0] == -24.0f);
   CHECK(blocks.custom.bbox.min_y[0] == -4.0f);
   CHECK(blocks.custom.bbox.min_z[0] == -24.0f);
   CHECK(blocks.custom.bbox.max_x[0] == 24.0f);
   CHECK(blocks.custom.bbox.max_y[0] == 4.0f);
   CHECK(blocks.custom.bbox.max_z[0] == 24.0f);
   CHECK(not blocks.custom.hidden[0]);
   CHECK(blocks.custom.layer[0] == 1);
   CHECK(blocks.custom.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[0].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[0].mesh_description == mesh_desc0);
   CHECK(blocks.custom.ids[0] == id0);

   CHECK(blocks.custom.bbox.min_x[1] == -1.0f);
   CHECK(blocks.custom.bbox.min_y[1] == -1.0f);
   CHECK(blocks.custom.bbox.min_z[1] == -1.0f);
   CHECK(blocks.custom.bbox.max_x[1] == 1.0f);
   CHECK(blocks.custom.bbox.max_y[1] == 1.0f);
   CHECK(blocks.custom.bbox.max_z[1] == 1.0f);
   CHECK(not blocks.custom.hidden[1]);
   CHECK(blocks.custom.layer[1] == 1);
   CHECK(blocks.custom.description[1].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[1].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[1].mesh_description == mesh_desc1);
   CHECK(blocks.custom.ids[1] == id2);

   CHECK(blocks.custom.bbox.min_x[2] == -1.0f);
   CHECK(blocks.custom.bbox.min_y[2] == -1.0f);
   CHECK(blocks.custom.bbox.min_z[2] == -1.0f);
   CHECK(blocks.custom.bbox.max_x[2] == 1.0f);
   CHECK(blocks.custom.bbox.max_y[2] == 1.0f);
   CHECK(blocks.custom.bbox.max_z[2] == 1.0f);
   CHECK(not blocks.custom.hidden[2]);
   CHECK(blocks.custom.layer[2] == 1);
   CHECK(blocks.custom.description[2].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[2].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[2].mesh_description == mesh_desc1);
   CHECK(blocks.custom.ids[2] == id4);

   REQUIRE(blocks.custom.dirty.size() == 1);
   CHECK(blocks.custom.dirty[0] == world::blocks_dirty_range{1, 3});

   CHECK(blocks.custom_meshes.debug_ref_count(mesh_desc0) == 1);
   CHECK(blocks.custom_meshes.debug_ref_count(mesh_desc1) == 2);

   world.blocks.untracked_clear_dirty_ranges();

   edit->revert(edit_context);

   REQUIRE(blocks.custom.size() == 5);
   CHECK(blocks.custom.is_balanced());

   CHECK(blocks.custom.bbox.min_x[0] == -24.0f);
   CHECK(blocks.custom.bbox.min_y[0] == -4.0f);
   CHECK(blocks.custom.bbox.min_z[0] == -24.0f);
   CHECK(blocks.custom.bbox.max_x[0] == 24.0f);
   CHECK(blocks.custom.bbox.max_y[0] == 4.0f);
   CHECK(blocks.custom.bbox.max_z[0] == 24.0f);
   CHECK(not blocks.custom.hidden[0]);
   CHECK(blocks.custom.layer[0] == 2);
   CHECK(blocks.custom.description[0].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[0].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[0].mesh_description == mesh_desc0);
   CHECK(blocks.custom.ids[0] == id0);

   CHECK(blocks.custom.bbox.min_x[1] == -24.0f);
   CHECK(blocks.custom.bbox.min_y[1] == -4.0f);
   CHECK(blocks.custom.bbox.min_z[1] == -24.0f);
   CHECK(blocks.custom.bbox.max_x[1] == 24.0f);
   CHECK(blocks.custom.bbox.max_y[1] == 4.0f);
   CHECK(blocks.custom.bbox.max_z[1] == 24.0f);
   CHECK(not blocks.custom.hidden[1]);
   CHECK(blocks.custom.layer[1] == 1);
   CHECK(blocks.custom.description[1].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[1].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[1].mesh_description == mesh_desc0);
   CHECK(blocks.custom.ids[1] == id1);

   CHECK(blocks.custom.bbox.min_x[2] == -1.0f);
   CHECK(blocks.custom.bbox.min_y[2] == -1.0f);
   CHECK(blocks.custom.bbox.min_z[2] == -1.0f);
   CHECK(blocks.custom.bbox.max_x[2] == 1.0f);
   CHECK(blocks.custom.bbox.max_y[2] == 1.0f);
   CHECK(blocks.custom.bbox.max_z[2] == 1.0f);
   CHECK(not blocks.custom.hidden[2]);
   CHECK(blocks.custom.layer[2] == 2);
   CHECK(blocks.custom.description[2].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[2].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[2].mesh_description == mesh_desc1);
   CHECK(blocks.custom.ids[2] == id2);

   CHECK(blocks.custom.bbox.min_x[3] == -1.0f);
   CHECK(blocks.custom.bbox.min_y[3] == -1.0f);
   CHECK(blocks.custom.bbox.min_z[3] == -1.0f);
   CHECK(blocks.custom.bbox.max_x[3] == 1.0f);
   CHECK(blocks.custom.bbox.max_y[3] == 1.0f);
   CHECK(blocks.custom.bbox.max_z[3] == 1.0f);
   CHECK(not blocks.custom.hidden[3]);
   CHECK(blocks.custom.layer[3] == 1);
   CHECK(blocks.custom.description[3].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[3].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[3].mesh_description == mesh_desc1);
   CHECK(blocks.custom.ids[3] == id3);

   CHECK(blocks.custom.bbox.min_x[4] == -1.0f);
   CHECK(blocks.custom.bbox.min_y[4] == -1.0f);
   CHECK(blocks.custom.bbox.min_z[4] == -1.0f);
   CHECK(blocks.custom.bbox.max_x[4] == 1.0f);
   CHECK(blocks.custom.bbox.max_y[4] == 1.0f);
   CHECK(blocks.custom.bbox.max_z[4] == 1.0f);
   CHECK(not blocks.custom.hidden[4]);
   CHECK(blocks.custom.layer[4] == 2);
   CHECK(blocks.custom.description[4].rotation == quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[4].position == float3{0.0f, 0.0f, 0.0f});
   CHECK(blocks.custom.description[4].mesh_description == mesh_desc1);
   CHECK(blocks.custom.ids[4] == id4);

   REQUIRE(blocks.custom.dirty.size() == 1);
   CHECK(blocks.custom.dirty[0] == world::blocks_dirty_range{1, 5});

   CHECK(blocks.custom_meshes.debug_ref_count(mesh_desc0) == 2);
   CHECK(blocks.custom_meshes.debug_ref_count(mesh_desc1) == 3);
}
}
