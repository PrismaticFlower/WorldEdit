#include "pch.h"

#include "edits/add_layer.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_layer", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_layer("Wow", world);

   action->apply(edit_context);

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   CHECK(world.layer_descriptions[1].name == "Wow");

   REQUIRE(world.common_layers.size() == 2);
   CHECK(world.common_layers[1] == 1);

   REQUIRE(world.requirements.size() == 1);
   CHECK(world.requirements[0].file_type == "world");
   REQUIRE(world.requirements[0].entries.size() == 2);
   CHECK(world.requirements[0].entries[0] == "Test");
   CHECK(world.requirements[0].entries[1] == "Test_Wow");

   action->revert(edit_context);

   REQUIRE(world.layer_descriptions.size() == 1);
   CHECK(world.layer_descriptions[0].name == "[Base]");

   REQUIRE(world.common_layers.size() == 1);
   CHECK(world.common_layers[0] == 0);

   REQUIRE(world.requirements == test_world.requirements);
}

TEST_CASE("edits add_layer empty req", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world.requirements.clear();

   auto action = make_add_layer("Wow", world);

   action->apply(edit_context);

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   CHECK(world.layer_descriptions[1].name == "Wow");

   REQUIRE(world.common_layers.size() == 2);
   CHECK(world.common_layers[1] == 1);

   REQUIRE(world.requirements.empty());

   action->revert(edit_context);

   REQUIRE(world.layer_descriptions.size() == 1);
   CHECK(world.layer_descriptions[0].name == "[Base]");

   REQUIRE(world.common_layers.size() == 1);
   CHECK(world.common_layers[0] == 0);

   REQUIRE(world.requirements.empty());
}

TEST_CASE("edits add_layer preivously deleted", "[Edits]")
{
   world::world test_world_preivously_deleted = test_world;

   test_world_preivously_deleted.deleted_layers = {"Now", "Wow", "Meow"};

   world::world world = test_world_preivously_deleted;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_layer("Wow", world);

   action->apply(edit_context);

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   CHECK(world.layer_descriptions[1].name == "Wow");

   REQUIRE(world.common_layers.size() == 2);
   CHECK(world.common_layers[1] == 1);

   REQUIRE(world.deleted_layers.size() == 2);
   CHECK(world.deleted_layers[0] == "Now");
   CHECK(world.deleted_layers[1] == "Meow");

   action->revert(edit_context);

   REQUIRE(world.layer_descriptions == test_world_preivously_deleted.layer_descriptions);
   REQUIRE(world.common_layers == test_world_preivously_deleted.common_layers);
   REQUIRE(world.requirements == test_world_preivously_deleted.requirements);
   REQUIRE(world.deleted_layers == test_world_preivously_deleted.deleted_layers);
}

}
