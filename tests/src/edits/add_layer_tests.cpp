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
   REQUIRE(world.layer_descriptions[0].name == "[Base]");
   REQUIRE(world.layer_descriptions[1].name == "Wow");

   action->revert(edit_context);

   REQUIRE(world.layer_descriptions.size() == 1);
   REQUIRE(world.layer_descriptions[0].name == "[Base]");
}

}
