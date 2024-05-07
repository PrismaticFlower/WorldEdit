#include "pch.h"

#include "edits/delete_animation_group.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world test_world_delete_animation_group = {
   .animation_groups = {
      world::entities_init,
      std::initializer_list{
         world::animation_group{.name = "Animation0"},
         world::animation_group{.name = "Animation1"},
         world::animation_group{.name = "Animation2"},
      },
   }};
}

TEST_CASE("edits delete_animation_group", "[Edits]")
{
   world::world world = test_world_delete_animation_group;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_animation_group(1);

   edit->apply(edit_context);

   REQUIRE(world.animation_groups.size() == 2);
   CHECK(world.animation_groups[0].name == "Animation0");
   CHECK(world.animation_groups[1].name == "Animation2");

   edit->revert(edit_context);

   REQUIRE(world.animation_groups.size() == 3);
   CHECK(world.animation_groups[0].name == "Animation0");
   CHECK(world.animation_groups[1].name == "Animation1");
   CHECK(world.animation_groups[2].name == "Animation2");
}
}
