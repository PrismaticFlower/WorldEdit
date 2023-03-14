#include "pch.h"

#include "edits/add_property.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_property path", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_property(world.paths[0].id, "AmazingProperty");

   action->apply(edit_context);

   REQUIRE(world.paths[0].properties.size() == 2);
   REQUIRE(world.paths[0].properties[0].key == "Key");
   REQUIRE(world.paths[0].properties[1].key == "AmazingProperty");

   action->revert(edit_context);

   REQUIRE(world.paths[0].properties.size() == 1);
   REQUIRE(world.paths[0].properties[0].key == "Key");
}

TEST_CASE("edits add_property path node", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_property(world.paths[0].id, 0, "AmazingProperty");

   action->apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].properties.size() == 2);
   REQUIRE(world.paths[0].nodes[0].properties[0].key == "Key");
   REQUIRE(world.paths[0].nodes[0].properties[1].key == "AmazingProperty");

   action->revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].properties.size() == 1);
   REQUIRE(world.paths[0].nodes[0].properties[0].key == "Key");
}

}
