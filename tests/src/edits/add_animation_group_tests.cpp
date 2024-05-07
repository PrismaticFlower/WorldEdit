#include "pch.h"

#include "edits/add_animation_group.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_animation_group", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_add_animation_group(world::animation_group{.name = "DoorOpen"});

   edit->apply(edit_context);

   REQUIRE(world.animation_groups.size() == 1);
   CHECK(world.animation_groups[0].name == "DoorOpen");

   edit->revert(edit_context);

   REQUIRE(world.animation_groups.size() == 0);
}
}
