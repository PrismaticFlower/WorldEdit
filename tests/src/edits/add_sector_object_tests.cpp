#include "pch.h"

#include "edits/add_sector_object.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_sector_object tests", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_sector_object(world.sectors[0].id, "such_an_object");

   action->apply(edit_context);

   REQUIRE(world.sectors[0].objects.size() == 2);
   CHECK(world.sectors[0].objects[0] == "test_object");
   CHECK(world.sectors[0].objects[1] == "such_an_object");

   action->revert(edit_context);

   REQUIRE(world.sectors[0].objects.size() == 1);
   CHECK(world.sectors[0].objects[0] == "test_object");
}

}
