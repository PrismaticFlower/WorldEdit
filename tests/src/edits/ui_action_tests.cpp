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
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action =
      std::make_unique<ui_edit<world::object, int>>(world.objects[0].id,
                                                    &world::object::team, 1,
                                                    world.objects[0].team);

   action->apply(edit_context);

   REQUIRE(world.objects[0].team == 1);

   action->revert(edit_context);

   REQUIRE(world.objects[0].team == 0);
}

TEST_CASE("edits ui_edit_indexed", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action =
      std::make_unique<ui_edit_indexed<world::object, world::instance_property>>(
         world.objects[0].id, &world::object::instance_properties, 0,
         world::instance_property{.key = "MaxHealth"s, .value = "10"s},
         world.objects[0].instance_properties[0]);

   action->apply(edit_context);

   REQUIRE(world.objects[0].instance_properties[0].value == "10");

   action->revert(edit_context);

   REQUIRE(world.objects[0].instance_properties[0].value == "50000");
}

TEST_CASE("edits ui_edit_path_node", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action =
      std::make_unique<ui_edit_path_node<float3>>(world.paths[0].id, 0,
                                                  &world::path::node::position,
                                                  float3{-1.0f, -1.0f, -1.0f},
                                                  world.paths[0].nodes[0].position);

   action->apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{-1.0f, -1.0f, -1.0f});

   action->revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits ui_edit_path_node_indexed", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = std::make_unique<ui_edit_path_node_indexed<world::path::property>>(
      world.paths[0].id, 0, &world::path::node::properties, 0,
      world::path::property{.key = "Key"s, .value = "NewValue"s},
      world.paths[0].nodes[0].properties[0]);

   action->apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].properties[0].value == "NewValue");

   action->revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].properties[0].value == "Value");
}

TEST_CASE("edits ui_creation_edit", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_edit edit{&world::object::layer, 1, 0};

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 1);

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
}

TEST_CASE("edits ui_creation_edit_with_meta", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_edit_with_meta edit{&world::object::layer,
                                   1,
                                   0,
                                   &world::edit_context::euler_rotation,
                                   {1.0f, 1.0f, 1.0f},
                                   {0.0f, 0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 1);
   REQUIRE(edit_context.euler_rotation == float3{1.0f, 1.0f, 1.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

}
