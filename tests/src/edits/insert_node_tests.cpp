#include "pch.h"

#include "edits/insert_node.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits insert_node", "[Edits]")
{
   world::world world = test_world;
   world::edit_context edit_context{world};

   world::path::node node{.position = {0.0f, 1.0f, 0.0f}};

   auto action = make_insert_node(world.paths[0].id, 0, node);

   action->apply(edit_context);

   REQUIRE(world.paths[0].nodes.size() == 2);
   REQUIRE(world.paths[0].nodes[0] == node);

   action->revert(edit_context);

   REQUIRE(world.paths[0].nodes.size() == 1);
   REQUIRE(world.paths[0].nodes[0] != node);
}
}
