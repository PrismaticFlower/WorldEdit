#include "pch.h"

#include "edits/add_animation_hierarchy.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_animation_hierarchy", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_add_animation_hierarchy(
      world::animation_hierarchy{.root_object = "door"});

   edit->apply(edit_context);

   REQUIRE(world.animation_hierarchies.size() == 1);
   CHECK(world.animation_hierarchies[0].root_object == "door");

   edit->revert(edit_context);

   REQUIRE(world.animation_hierarchies.size() == 0);
}
}
