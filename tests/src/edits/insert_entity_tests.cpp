#include "pch.h"

#include "edits/insert_entity.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits insert_entity", "[Edits]")
{
   world::world world = test_world;

   world::boundary boundary{.name = "Boundary2", .id = world::boundary_id{1}};

   auto action = make_insert_entity(boundary);

   action->apply(world);

   REQUIRE(world.boundaries.size() == 2);
   REQUIRE(world.boundaries[1].name == "Boundary2");

   action->revert(world);

   REQUIRE(world.boundaries.size() == 1);
   REQUIRE(world.boundaries[0].name != "Boundary2");
}
}
