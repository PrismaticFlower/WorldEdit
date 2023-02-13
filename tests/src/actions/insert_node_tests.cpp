#include "pch.h"

#include "actions/insert_node.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::actions::tests {

TEST_CASE("actions insert_node", "[Actions]")
{
   world::world world = test_world;

   world::path::node node{.position = {0.0f, 1.0f, 0.0f}};

   auto action = make_insert_node(world.paths[0].id, 0, node);

   action->apply(world);

   REQUIRE(world.paths[0].nodes.size() == 2);
   REQUIRE(world.paths[0].nodes[0] == node);

   action->revert(world);

   REQUIRE(world.paths[0].nodes.size() == 1);
   REQUIRE(world.paths[0].nodes[0] != node);
}
}
