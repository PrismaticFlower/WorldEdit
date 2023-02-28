#include "pch.h"

#include "edits/insert_point.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits insert_point", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   const float2 point = {1e10f, 0.0f};

   auto action = make_insert_point(world.sectors[0].id, 0, point);

   action->apply(edit_context);

   REQUIRE(world.sectors[0].points.size() == 5);
   REQUIRE(world.sectors[0].points[0] == point);

   action->revert(edit_context);

   REQUIRE(world.sectors[0].points.size() == 4);
   REQUIRE(world.sectors[0].points[0] != point);
}
}
