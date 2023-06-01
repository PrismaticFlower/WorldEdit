#include "pch.h"

#include "edits/delete_sector_object.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits delete_sector_object", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_sector_object(world.sectors[0].id, 0, world);

   edit->apply(edit_context);

   CHECK(world.sectors[0].objects.empty());

   edit->revert(edit_context);

   REQUIRE(world.sectors[0].objects.size() == 1);
   CHECK(world.sectors[0].objects[0] == "test_object");
}

}
