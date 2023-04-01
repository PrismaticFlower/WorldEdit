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

   auto action = make_add_layer("Wow");

   action->apply(edit_context);

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   CHECK(world.layer_descriptions[1].name == "Wow");

   REQUIRE(world.game_modes[0].layers.size() == 2);
   CHECK(world.game_modes[0].layers[1] == 1);

   REQUIRE(world.requirements.size() == 1);
   CHECK(world.requirements[0].file_type == "world");
   REQUIRE(world.requirements[0].entries.size() == 2);
   CHECK(world.requirements[0].entries[0] == "Test");
   CHECK(world.requirements[0].entries[1] == "Test_Wow");

   action->revert(edit_context);

   REQUIRE(world.layer_descriptions.size() == 1);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   REQUIRE(world.game_modes[0].layers.size() == 1);

   REQUIRE(world.requirements == test_world.requirements);
}

TEST_CASE("edits add_layer empty req", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world.requirements.clear();

   auto action = make_add_layer("Wow");

   action->apply(edit_context);

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   CHECK(world.layer_descriptions[1].name == "Wow");

   REQUIRE(world.game_modes[0].layers.size() == 2);
   CHECK(world.game_modes[0].layers[1] == 1);

   REQUIRE(world.requirements.empty());

   action->revert(edit_context);

   REQUIRE(world.layer_descriptions.size() == 1);
   CHECK(world.layer_descriptions[0].name == "[Base]");
   REQUIRE(world.game_modes[0].layers.size() == 1);

   REQUIRE(world.requirements.empty());
}

}
