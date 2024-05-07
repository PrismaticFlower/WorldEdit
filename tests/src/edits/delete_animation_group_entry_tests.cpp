#include "pch.h"

#include "edits/delete_animation_group_entry.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world test_world_delete_animation_group_entry = {
   .animation_groups = {
      world::entities_init,
      std::initializer_list{
         world::animation_group{
            .name = "Animation",
            .entries =
               {
                  {"Animation0", "Object0"},
                  {"Animation1", "Object1"},
                  {"Animation2", "Object2"},
               },
         },
      },
   }};
}

TEST_CASE("edits delete_animation_group_entry", "[Edits]")
{
   world::world world = test_world_delete_animation_group_entry;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_delete_animation_group_entry(&world.animation_groups[0].entries, 1);

   edit->apply(edit_context);

   REQUIRE(world.animation_groups[0].entries.size() == 2);
   CHECK(world.animation_groups[0].entries[0].animation == "Animation0");
   CHECK(world.animation_groups[0].entries[1].animation == "Animation2");

   edit->revert(edit_context);

   REQUIRE(world.animation_groups[0].entries.size() == 3);
   CHECK(world.animation_groups[0].entries[0].animation == "Animation0");
   CHECK(world.animation_groups[0].entries[1].animation == "Animation1");
   CHECK(world.animation_groups[0].entries[2].animation == "Animation2");
}
}
