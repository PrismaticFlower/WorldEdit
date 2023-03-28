#include "pch.h"

#include "edits/delete_layer.hpp"
#include "world/world.hpp"

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

   .layer_descriptions = {{.name = "[Base]"}, {.name = "Middle"}, {.name = "Top"}},

   .objects =
      {
         object{.name = "Object0", .layer = 0},
         object{.name = "Object1", .layer = 1},
         object{.name = "Object2", .layer = 2},
         object{.name = "Object3", .layer = 1},
         object{.name = "Object4", .layer = 2},
      },

   .lights =
      {
         light{.name = "Light0", .layer = 0},
         light{.name = "Light1", .layer = 1},
         light{.name = "Light2", .layer = 2},
      },

   .paths =
      {
         path{.name = "Path0", .layer = 0},
         path{.name = "Path1", .layer = 1},
         path{.name = "Path2", .layer = 2},
      },

   .regions =
      {
         region{.name = "Region0", .layer = 0},
         region{.name = "Region1", .layer = 1},
         region{.name = "Region2", .layer = 2},
      },

   .hintnodes =
      {
         hintnode{.name = "Hintnode0", .layer = 0},
         hintnode{.name = "Hintnode1", .layer = 1},
         hintnode{.name = "Hintnode2", .layer = 2},
      },
};

}

TEST_CASE("edits delete_layer", "[Edits]")
{
   world::world world = layer_delete_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_delete_layer(1, world);

   action->apply(edit_context);

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   CHECK(world.layer_descriptions[1].name == "Top");

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

   action->revert(edit_context);

   CHECK(world.layer_descriptions == layer_delete_test_world.layer_descriptions);
   CHECK(world.objects == layer_delete_test_world.objects);
   CHECK(world.lights == layer_delete_test_world.lights);
   CHECK(world.paths == layer_delete_test_world.paths);
   CHECK(world.regions == layer_delete_test_world.regions);
   CHECK(world.hintnodes == layer_delete_test_world.hintnodes);
}

}
