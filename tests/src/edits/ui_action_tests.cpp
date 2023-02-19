#include "pch.h"

#include "edits/ui_action.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

#include <memory>

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits ui_edit", "[Edits]")
{
   world::world world = test_world;

   auto action =
      std::make_unique<ui_edit<world::object, int>>(world.objects[0].id,
                                                    &world::object::team, 1,
                                                    world.objects[0].team);

   action->apply(world);

   REQUIRE(world.objects[0].team == 1);

   action->revert(world);

   REQUIRE(world.objects[0].team == 0);
}

TEST_CASE("edits ui_edit_indexed", "[Edits]")
{
   world::world world = test_world;

   auto action =
      std::make_unique<ui_edit_indexed<world::object, world::instance_property>>(
         world.objects[0].id, &world::object::instance_properties, 0,
         world::instance_property{.key = "MaxHealth"s, .value = "10"s},
         world.objects[0].instance_properties[0]);

   action->apply(world);

   REQUIRE(world.objects[0].instance_properties[0].value == "10");

   action->revert(world);

   REQUIRE(world.objects[0].instance_properties[0].value == "50000");
}

TEST_CASE("edits ui_edit_path_node", "[Edits]")
{
   world::world world = test_world;

   auto action =
      std::make_unique<ui_edit_path_node<float3>>(world.paths[0].id, 0,
                                                  &world::path::node::position,
                                                  float3{-1.0f, -1.0f, -1.0f},
                                                  world.paths[0].nodes[0].position);

   action->apply(world);

   REQUIRE(world.paths[0].nodes[0].position == float3{-1.0f, -1.0f, -1.0f});

   action->revert(world);

   REQUIRE(world.paths[0].nodes[0].position == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits ui_edit_path_node_indexed", "[Edits]")
{
   world::world world = test_world;

   auto action = std::make_unique<ui_edit_path_node_indexed<world::path::property>>(
      world.paths[0].id, 0, &world::path::node::properties, 0,
      world::path::property{.key = "Key"s, .value = "NewValue"s},
      world.paths[0].nodes[0].properties[0]);

   action->apply(world);

   REQUIRE(world.paths[0].nodes[0].properties[0].value == "NewValue");

   action->revert(world);

   REQUIRE(world.paths[0].nodes[0].properties[0].value == "Value");
}

}
