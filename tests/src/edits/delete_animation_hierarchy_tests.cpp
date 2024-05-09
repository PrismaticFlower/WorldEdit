#include "pch.h"

#include "edits/delete_animation_hierarchy.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world test_world_delete_animation_hierarchy = {
   .animation_hierarchies = {
      world::entities_init,
      std::initializer_list{
         world::animation_hierarchy{.root_object = "object_0"},
         world::animation_hierarchy{.root_object = "object_1"},
         world::animation_hierarchy{.root_object = "object_2"},
      },
   }};

}

TEST_CASE("edits delete_animation_hierarchy", "[Edits]")
{
   world::world world = test_world_delete_animation_hierarchy;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_animation_hierarchy(1);

   edit->apply(edit_context);

   REQUIRE(world.animation_hierarchies.size() == 2);
   CHECK(world.animation_hierarchies[0].root_object == "object_0");
   CHECK(world.animation_hierarchies[1].root_object == "object_2");

   edit->revert(edit_context);

   REQUIRE(world.animation_hierarchies.size() == 3);
   CHECK(world.animation_hierarchies[0].root_object == "object_0");
   CHECK(world.animation_hierarchies[1].root_object == "object_1");
   CHECK(world.animation_hierarchies[2].root_object == "object_2");
}
}
