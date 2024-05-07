#include "pch.h"

#include "edits/add_animation_group_entry.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world test_world_add_animation_group_entry = {
   .animation_groups = {
      world::entities_init,
      std::initializer_list{
         world::animation_group{
            .name = "Animation",
         },
      },
   }};
}

TEST_CASE("edits add_animation_group_entry", "[Edits]")
{
   world::world world = test_world_add_animation_group_entry;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_add_animation_group_entry(&world.animation_groups[0].entries,
                                     {.animation = "DoorOpen", .object = "Door"});

   edit->apply(edit_context);

   REQUIRE(world.animation_groups[0].entries.size() == 1);
   CHECK(world.animation_groups[0].entries[0].animation == "DoorOpen");
   CHECK(world.animation_groups[0].entries[0].object == "Door");

   edit->revert(edit_context);

   REQUIRE(world.animation_groups[0].entries.size() == 0);
}
}
