#include "pch.h"

#include "edits/add_animation_hierarchy_child.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world test_world_add_animation_hierarchy_child = {
   .animation_hierarchies = {
      world::entities_init,
      std::initializer_list{
         world::animation_hierarchy{
            .root_object = "ship",
         },
      },
   }};
}

TEST_CASE("edits add_animation_hierarchy_child", "[Edits]")
{
   world::world world = test_world_add_animation_hierarchy_child;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_add_animation_hierarchy_child(&world.animation_hierarchies[0].objects, "Gun0");

   edit->apply(edit_context);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 1);
   CHECK(world.animation_hierarchies[0].objects[0] == "Gun0");

   edit->revert(edit_context);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 0);
}
}
