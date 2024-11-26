#include "pch.h"

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

}
