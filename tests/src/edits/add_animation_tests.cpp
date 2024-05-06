#include "pch.h"

#include "edits/add_animation.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_animation", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_add_animation(world::animation{.name = "DoorOpen"});

   edit->apply(edit_context);

   REQUIRE(world.animations.size() == 1);
   CHECK(world.animations[0].name == "DoorOpen");

   edit->revert(edit_context);

   REQUIRE(world.animations.size() == 0);
}
}
