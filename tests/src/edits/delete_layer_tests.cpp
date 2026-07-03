#include "pch.h"

#include "edits/add_block.hpp"
#include "edits/delete_layer.hpp"
#include "world/object_class_library.hpp"
#include "world/world.hpp"

#include "null_asset_libraries.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

using world::animation_group;
using world::animation_hierarchy;
using world::hintnode;
using world::light;
using world::object;
using world::path;
using world::region;
using world::sector;
using world::tree_line;
using world::tree_line_odf;

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

TEST_CASE("edits delete_layer tree line", "[Edits]")
{
   world::object_class_library object_class_library{null_asset_libraries()};
   world::world world = {
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

      .paths = {world::entities_init,
                std::initializer_list{
                   path{.name = "Path0", .layer = 0},
                }},

      .tree_lines =
         {
            pinned_vector_init{.max_size = world::max_tree_lines},
            std::initializer_list{
               tree_line{
                  .path_index = 0,
               },
            },
         },
   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_layer(0, world, object_class_library);

   edit->apply(edit_context);

   CHECK(world.tree_lines.size() == 0);

   edit->revert(edit_context);

   REQUIRE(world.tree_lines.size() == 1);
   CHECK(world.tree_lines[0].path_index == 0);
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

TEST_CASE("edits delete_layer tree line class handle liftime", "[Edits]")
{
   world::object_class_library object_class_library{null_asset_libraries()};
   world::world world = {
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

      .paths = {world::entities_init,
                std::initializer_list{
                   path{.name = "Path0", .layer = 0},
                }},

      .tree_lines =
         {
            pinned_vector_init{.max_size = world::max_tree_lines},
            std::initializer_list{
               tree_line{
                  .border_odfs =
                     {
                        tree_line_odf{
                           .name = "tree1",
                           .handle = object_class_library.acquire(lowercase_string{"tree1"sv}),
                        },
                        tree_line_odf{
                           .name = "tree2",
                           .handle = object_class_library.acquire(lowercase_string{"tree2"sv}),
                        },
                        tree_line_odf{
                           .name = "tree1",
                           .handle = object_class_library.acquire(lowercase_string{"tree1"sv}),
                        },
                        tree_line_odf{
                           .name = "tree3",
                           .handle = object_class_library.acquire(lowercase_string{"tree3"sv}),
                        },
                     },
                  .path_index = 0,
               },
            },
         },
   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_layer(0, world, object_class_library);

   edit->apply(edit_context);

   CHECK(object_class_library.debug_ref_count(lowercase_string{"tree1"sv}) == 0);
   CHECK(object_class_library.debug_ref_count(lowercase_string{"tree2"sv}) == 0);
   CHECK(object_class_library.debug_ref_count(lowercase_string{"tree3"sv}) == 0);

   edit->revert(edit_context);

   CHECK(object_class_library.debug_ref_count(lowercase_string{"tree1"sv}) == 2);
   CHECK(object_class_library.debug_ref_count(lowercase_string{"tree2"sv}) == 1);
   CHECK(object_class_library.debug_ref_count(lowercase_string{"tree3"sv}) == 1);
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

TEST_CASE("edits delete_layer unlink objects", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .requirements = {{.file_type = "world",
                        .entries = {"Test", "Test_Middle", "Test_Top", "Test_Unlinked"}}},

      .layer_descriptions = {{.name = "[Base]"},
                             {.name = "Middle"},
                             {.name = "Top"},
                             {.name = "Unlinked"}},
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
      .common_layers = {0, 2, 3},

      .objects = {world::entities_init,
                  std::initializer_list{
                     object{.name = "Object0", .layer = 3},
                     object{.name = "Object1", .layer = 1},
                     object{.name = "Object2", .layer = 2},
                     object{.name = "Object3",
                            .layer = 1,
                            .instance_properties = {{"ControlZone",
                                                     "Object4"}}},
                     object{.name = "Object4", .layer = 3},
                  }},

      .paths = {world::entities_init,
                std::initializer_list{
                   path{.name = "Path0", .layer = 0, .properties = {{"EnableObject", "Object4"}}},
                   path{.name = "Path1", .layer = 1},
                   path{.name = "Path2", .layer = 2},
                }},

      .sectors =
         {
            {.max_size = 1},
            std::initializer_list{
               sector{
                  .name = "Sector",

                  .objects = {4, 1},
               },
            },
         },

      .hintnodes = {world::entities_init,
                    std::initializer_list{
                       hintnode{.name = "Hintnode0", .layer = 0},
                       hintnode{.name = "Hintnode1", .layer = 1},
                       hintnode{.name = "Hintnode2", .layer = 2, .command_post = 4},
                    }},

      .animation_groups =
         {
            {.max_size = 1},
            std::initializer_list{
               animation_group{
                  .name = "Group",
                  .entries =
                     {
                        animation_group::entry{.animation_index = 0, .object_index = 4},
                        animation_group::entry{.animation_index = 0, .object_index = 1},
                     },
               },
            },
         },

      .animation_hierarchies =
         {
            {.max_size = 1},
            std::initializer_list{
               animation_hierarchy{
                  .root_object = 3,

                  .objects = {4, 1},
               },
               animation_hierarchy{
                  .root_object = 4,
               },
            },
         },
   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_layer(3, world, object_class_library);

   edit->apply(edit_context);

   CHECK(world.objects[2].instance_properties[0].value == "");

   CHECK(world.paths[0].properties.size() == 0);

   REQUIRE(world.sectors[0].objects.size() == 1);
   CHECK(world.sectors[0].objects[0] == 0);

   CHECK(not world.hintnodes[2].command_post.has_index());

   REQUIRE(world.animation_groups[0].entries.size() == 1);
   CHECK(world.animation_groups[0].entries[0].object_index == 0);

   REQUIRE(world.animation_hierarchies.size() == 1);
   CHECK(world.animation_hierarchies[0].root_object == 2);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 1);
   CHECK(world.animation_hierarchies[0].objects[0] == 0);

   edit->revert(edit_context);

   CHECK(world.objects[3].instance_properties[0].value == "Object4");

   CHECK(world.paths[0].properties.size() == 1);
   CHECK(world.paths[0].properties[0].key == "EnableObject");
   CHECK(world.paths[0].properties[0].value == "Object4");

   REQUIRE(world.sectors[0].objects.size() == 2);
   CHECK(world.sectors[0].objects[0] == 4);
   CHECK(world.sectors[0].objects[1] == 1);

   CHECK(world.hintnodes[2].command_post == 4);

   REQUIRE(world.animation_groups[0].entries.size() == 2);
   CHECK(world.animation_groups[0].entries[0].object_index == 4);
   CHECK(world.animation_groups[0].entries[1].object_index == 1);

   REQUIRE(world.animation_hierarchies.size() == 2);

   CHECK(world.animation_hierarchies[0].root_object == 3);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 2);
   CHECK(world.animation_hierarchies[0].objects[0] == 4);
   CHECK(world.animation_hierarchies[0].objects[1] == 1);

   CHECK(world.animation_hierarchies[1].root_object == 4);
}

TEST_CASE("edits delete_layer adjust object indices", "[Edits]")
{
   world::world
      world =
         {
            .name = "Test"s,

            .requirements = {{.file_type = "world",
                              .entries = {"Test", "Test_Middle", "Test_Top",
                                          "Test_Unlinked"}}},

            .layer_descriptions = {{.name = "[Base]"},
                                   {.name = "Middle"},
                                   {.name = "Top"},
                                   {.name = "Unlinked"}},
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
            .common_layers = {0, 2, 3},

            .objects = {world::entities_init,
                        std::initializer_list{
                           object{.name = "Object0", .layer = 3},
                           object{.name = "Object1", .layer = 3},
                           object{.name = "Object2", .layer = 2},
                           object{.name = "Object3", .layer = 1},
                           object{.name = "Object4", .layer = 3},
                           object{.name = "Object5", .layer = 3},
                           object{.name = "Object6", .layer = 3},
                           object{.name = "Object7", .layer = 1},
                           object{.name = "Object8", .layer = 1},
                           object{.name = "Object9", .layer = 1},
                        }},

            .sectors =
               {
                  {.max_size = 1},
                  std::initializer_list{
                     sector{
                        .name = "Sector",

                        .objects = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                     },
                  },
               },

            .hintnodes =
               {world::entities_init,
                std::initializer_list{
                   hintnode{.name = "Hintnode0", .layer = 3, .command_post = 0},
                   hintnode{.name = "Hintnode1", .layer = 3, .command_post = 1},
                   hintnode{.name = "Hintnode2", .layer = 2, .command_post = 2},
                   hintnode{.name = "Hintnode3", .layer = 1, .command_post = 3},
                   hintnode{.name = "Hintnode4", .layer = 3, .command_post = 4},
                   hintnode{.name = "Hintnode5", .layer = 3, .command_post = 5},
                   hintnode{.name = "Hintnode6", .layer = 3, .command_post = 6},
                   hintnode{.name = "Hintnode7", .layer = 1, .command_post = 7},
                   hintnode{.name = "Hintnode8", .layer = 1, .command_post = 8},
                   hintnode{.name = "Hintnode9", .layer = 1, .command_post = 9},
                }},

            .animation_groups =
               {
                  {.max_size = 1},
                  std::initializer_list{
                     animation_group{
                        .name = "Group",
                        .entries =
                           {
                              animation_group::entry{.animation_index = 0, .object_index = 0},
                              animation_group::entry{.animation_index = 0, .object_index = 1},
                              animation_group::entry{.animation_index = 0, .object_index = 2},
                              animation_group::entry{.animation_index = 0, .object_index = 3},
                              animation_group::entry{.animation_index = 0, .object_index = 4},
                              animation_group::entry{.animation_index = 0, .object_index = 5},
                              animation_group::entry{.animation_index = 0, .object_index = 6},
                              animation_group::entry{.animation_index = 0, .object_index = 7},
                              animation_group::entry{.animation_index = 0, .object_index = 8},
                              animation_group::entry{.animation_index = 0, .object_index = 9},
                           },
                     },
                  },
               },

            .animation_hierarchies =
               {
                  {.max_size = 1},
                  std::initializer_list{
                     animation_hierarchy{
                        .root_object = 3,

                        .objects = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                     },
                  },
               },
         };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_layer(3, world, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.sectors[0].objects.size() == 5);
   CHECK(world.sectors[0].objects[0] == 0);
   CHECK(world.sectors[0].objects[1] == 1);
   CHECK(world.sectors[0].objects[2] == 2);
   CHECK(world.sectors[0].objects[3] == 3);
   CHECK(world.sectors[0].objects[4] == 4);

   REQUIRE(world.hintnodes.size() == 5);
   CHECK(world.hintnodes[0].command_post == 0);
   CHECK(world.hintnodes[1].command_post == 1);
   CHECK(world.hintnodes[2].command_post == 2);
   CHECK(world.hintnodes[3].command_post == 3);
   CHECK(world.hintnodes[4].command_post == 4);

   REQUIRE(world.animation_groups[0].entries.size() == 5);
   CHECK(world.animation_groups[0].entries[0].object_index == 0);
   CHECK(world.animation_groups[0].entries[1].object_index == 1);
   CHECK(world.animation_groups[0].entries[2].object_index == 2);
   CHECK(world.animation_groups[0].entries[3].object_index == 3);
   CHECK(world.animation_groups[0].entries[4].object_index == 4);

   CHECK(world.animation_hierarchies[0].root_object == 1);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 5);
   CHECK(world.animation_hierarchies[0].objects[0] == 0);
   CHECK(world.animation_hierarchies[0].objects[1] == 1);
   CHECK(world.animation_hierarchies[0].objects[2] == 2);
   CHECK(world.animation_hierarchies[0].objects[3] == 3);
   CHECK(world.animation_hierarchies[0].objects[4] == 4);

   edit->revert(edit_context);

   REQUIRE(world.sectors[0].objects.size() == 10);
   CHECK(world.sectors[0].objects[0] == 0);
   CHECK(world.sectors[0].objects[1] == 1);
   CHECK(world.sectors[0].objects[2] == 2);
   CHECK(world.sectors[0].objects[3] == 3);
   CHECK(world.sectors[0].objects[4] == 4);
   CHECK(world.sectors[0].objects[5] == 5);
   CHECK(world.sectors[0].objects[6] == 6);
   CHECK(world.sectors[0].objects[7] == 7);
   CHECK(world.sectors[0].objects[8] == 8);
   CHECK(world.sectors[0].objects[9] == 9);

   REQUIRE(world.hintnodes.size() == 10);
   CHECK(world.hintnodes[0].command_post == 0);
   CHECK(world.hintnodes[1].command_post == 1);
   CHECK(world.hintnodes[2].command_post == 2);
   CHECK(world.hintnodes[3].command_post == 3);
   CHECK(world.hintnodes[4].command_post == 4);
   CHECK(world.hintnodes[5].command_post == 5);
   CHECK(world.hintnodes[6].command_post == 6);
   CHECK(world.hintnodes[7].command_post == 7);
   CHECK(world.hintnodes[8].command_post == 8);
   CHECK(world.hintnodes[9].command_post == 9);

   REQUIRE(world.animation_groups[0].entries.size() == 10);
   CHECK(world.animation_groups[0].entries[0].object_index == 0);
   CHECK(world.animation_groups[0].entries[1].object_index == 1);
   CHECK(world.animation_groups[0].entries[2].object_index == 2);
   CHECK(world.animation_groups[0].entries[3].object_index == 3);
   CHECK(world.animation_groups[0].entries[4].object_index == 4);
   CHECK(world.animation_groups[0].entries[5].object_index == 5);
   CHECK(world.animation_groups[0].entries[6].object_index == 6);
   CHECK(world.animation_groups[0].entries[7].object_index == 7);
   CHECK(world.animation_groups[0].entries[8].object_index == 8);
   CHECK(world.animation_groups[0].entries[9].object_index == 9);

   CHECK(world.animation_hierarchies[0].root_object == 3);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 10);
   CHECK(world.animation_hierarchies[0].objects[0] == 0);
   CHECK(world.animation_hierarchies[0].objects[1] == 1);
   CHECK(world.animation_hierarchies[0].objects[2] == 2);
   CHECK(world.animation_hierarchies[0].objects[3] == 3);
   CHECK(world.animation_hierarchies[0].objects[4] == 4);
   CHECK(world.animation_hierarchies[0].objects[5] == 5);
   CHECK(world.animation_hierarchies[0].objects[6] == 6);
   CHECK(world.animation_hierarchies[0].objects[7] == 7);
   CHECK(world.animation_hierarchies[0].objects[8] == 8);
   CHECK(world.animation_hierarchies[0].objects[9] == 9);
}

TEST_CASE("edits delete_layer adjust path indices", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .requirements = {{.file_type = "world",
                        .entries = {"Test", "Test_Middle", "Test_Top", "Test_Unlinked"}}},

      .layer_descriptions = {{.name = "[Base]"},
                             {.name = "Middle"},
                             {.name = "Top"},
                             {.name = "Unlinked"}},
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
      .common_layers = {0, 2, 3},

      .paths = {world::entities_init,
                std::initializer_list{
                   path{.name = "Path0", .layer = 3},
                   path{.name = "Path1", .layer = 3},
                   path{.name = "Path2", .layer = 2},
                   path{.name = "Path3", .layer = 1},
                   path{.name = "Path4", .layer = 3},
                   path{.name = "Path5", .layer = 3},
                   path{.name = "Path6", .layer = 3},
                   path{.name = "Path7", .layer = 1},
                   path{.name = "Path8", .layer = 1},
                   path{.name = "Path9", .layer = 1},
                }},

      .tree_lines =
         {
            {.max_size = 10},
            std::initializer_list{
               tree_line{.path_index = 0},
               tree_line{.path_index = 1},
               tree_line{.path_index = 2},
               tree_line{.path_index = 3},
               tree_line{.path_index = 4},
               tree_line{.path_index = 5},
               tree_line{.path_index = 6},
               tree_line{.path_index = 7},
               tree_line{.path_index = 8},
               tree_line{.path_index = 9},
            },
         },
   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_layer(3, world, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.tree_lines.size() == 5);
   CHECK(world.tree_lines[0].path_index == 0);
   CHECK(world.tree_lines[1].path_index == 1);
   CHECK(world.tree_lines[2].path_index == 2);
   CHECK(world.tree_lines[3].path_index == 3);
   CHECK(world.tree_lines[4].path_index == 4);

   edit->revert(edit_context);

   REQUIRE(world.tree_lines.size() == 10);
   CHECK(world.tree_lines[0].path_index == 0);
   CHECK(world.tree_lines[1].path_index == 1);
   CHECK(world.tree_lines[2].path_index == 2);
   CHECK(world.tree_lines[3].path_index == 3);
   CHECK(world.tree_lines[4].path_index == 4);
   CHECK(world.tree_lines[5].path_index == 5);
   CHECK(world.tree_lines[6].path_index == 6);
   CHECK(world.tree_lines[7].path_index == 7);
   CHECK(world.tree_lines[8].path_index == 8);
   CHECK(world.tree_lines[9].path_index == 9);
}

TEST_CASE("edits delete_layer unlink paths", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .requirements = {{.file_type = "world",
                        .entries = {"Test", "Test_Middle", "Test_Top", "Test_Unlinked"}}},

      .layer_descriptions = {{.name = "[Base]"},
                             {.name = "Middle"},
                             {.name = "Top"},
                             {.name = "Unlinked"}},
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
      .common_layers = {0, 2, 3},

      .objects = {world::entities_init,
                  std::initializer_list{
                     object{.name = "Object0",
                            .layer = 1,
                            .instance_properties =
                               {
                                  {"SpawnPath", "Path0"},
                                  {"AllyPath", "Path0"},
                                  {"TurretPath", "Path0"},
                               }},
                  }},

      .paths = {world::entities_init,
                std::initializer_list{
                   path{.name = "Path0", .layer = 3},
                }},

   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_layer(3, world, object_class_library);

   edit->apply(edit_context);

   CHECK(world.objects[0].instance_properties[0].value == "");
   CHECK(world.objects[0].instance_properties[1].value == "");
   CHECK(world.objects[0].instance_properties[2].value == "");

   edit->revert(edit_context);

   CHECK(world.objects[0].instance_properties[0].value == "Path0");
   CHECK(world.objects[0].instance_properties[1].value == "Path0");
   CHECK(world.objects[0].instance_properties[2].value == "Path0");
}

TEST_CASE("edits delete_layer unlink regions", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .requirements = {{.file_type = "world",
                        .entries = {"Test", "Test_Middle", "Test_Top", "Test_Unlinked"}}},

      .layer_descriptions = {{.name = "[Base]"},
                             {.name = "Middle"},
                             {.name = "Top"},
                             {.name = "Unlinked"}},
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
      .common_layers = {0, 2, 3},

      .objects = {world::entities_init,
                  std::initializer_list{
                     object{.name = "Object0",
                            .layer = 1,
                            .instance_properties =
                               {
                                  {"CaptureRegion", "Region0"},
                                  {"ControlRegion", "Region0"},
                                  {"EffectRegion", "Region0"},
                                  {"KillRegion", "Region0"},
                                  {"SpawnRegion", "Region0"},
                               }},
                  }},

      .regions = {world::entities_init,
                  std::initializer_list{
                     region{.name = "Region", .layer = 3, .description = "Region0"},
                  }},

   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_layer(3, world, object_class_library);

   edit->apply(edit_context);

   CHECK(world.objects[0].instance_properties[0].value == "");
   CHECK(world.objects[0].instance_properties[1].value == "");
   CHECK(world.objects[0].instance_properties[2].value == "");
   CHECK(world.objects[0].instance_properties[3].value == "");
   CHECK(world.objects[0].instance_properties[4].value == "");

   edit->revert(edit_context);

   CHECK(world.objects[0].instance_properties[0].value == "Region0");
   CHECK(world.objects[0].instance_properties[1].value == "Region0");
   CHECK(world.objects[0].instance_properties[2].value == "Region0");
   CHECK(world.objects[0].instance_properties[3].value == "Region0");
   CHECK(world.objects[0].instance_properties[4].value == "Region0");
}

TEST_CASE("edits delete_layer unlink lights", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .requirements = {{.file_type = "world",
                        .entries = {"Test", "Test_Middle", "Test_Top", "Test_Unlinked"}}},

      .layer_descriptions = {{.name = "[Base]"},
                             {.name = "Middle"},
                             {.name = "Top"},
                             {.name = "Unlinked"}},
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
      .common_layers = {0, 2, 3},

      .global_lights = {.global_light_1 = "Light1", .global_light_2 = "Light0"},

      .lights = {world::entities_init,
                 std::initializer_list{
                    light{.name = "Light0", .layer = 3},
                    light{.name = "Light1", .layer = 3},
                 }},
   };

   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_layer(3, world, object_class_library);

   edit->apply(edit_context);

   CHECK(world.global_lights.global_light_1 == "");
   CHECK(world.global_lights.global_light_2 == "");

   edit->revert(edit_context);

   CHECK(world.global_lights.global_light_1 == "Light1");
   CHECK(world.global_lights.global_light_2 == "Light0");
}

}
