#include "pch.h"

#include "edits/delete_animation.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world
   test_world_delete_animation =
      {
         .animations =
            {
               world::entities_init,
               std::initializer_list{
                  world::animation{
                     .name = "Animation0",
                     .id = world::animation_id{0},
                  },
                  world::animation{
                     .name = "Animation1",
                     .id = world::animation_id{1},
                  },
                  world::animation{
                     .name = "Animation2",
                     .id = world::animation_id{2},
                  },
               },
            },

         .animation_groups =
            {
               world::entities_init,
               std::initializer_list{
                  world::animation_group{
                     .name = "Animation0",
                     .entries =
                        {
                           {"Animation0", "object0"},
                           {"Animation1", "object1"},
                           {"Animation2", "object2"},
                        },
                     .id = world::animation_group_id{0},
                  },
                  world::animation_group{
                     .name = "Animation1",
                     .entries =
                        {
                           {"Animation1", "object1"},
                           {"Animation2", "object2"},
                        },
                     .id = world::animation_group_id{1},
                  },
                  world::animation_group{
                     .name = "Animation2",
                     .entries =
                        {
                           {"animation2", "object2"},
                           {"animation1", "object1"},
                        },
                     .id = world::animation_group_id{2},
                  },
               },
            },
};
}

TEST_CASE("edits delete_animation", "[Edits]")
{
   world::world world = test_world_delete_animation;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_animation(1, world);

   edit->apply(edit_context);

   REQUIRE(world.animations.size() == 2);
   CHECK(world.animations[0].name == "Animation0");
   CHECK(world.animations[1].name == "Animation2");

   REQUIRE(world.animation_groups[0].entries.size() == 2);
   CHECK(world.animation_groups[0].entries[0].animation == "Animation0");
   CHECK(world.animation_groups[0].entries[1].animation == "Animation2");

   REQUIRE(world.animation_groups[1].entries.size() == 1);
   CHECK(world.animation_groups[1].entries[0].animation == "Animation2");

   REQUIRE(world.animation_groups[2].entries.size() == 1);
   CHECK(world.animation_groups[2].entries[0].animation == "animation2");

   edit->revert(edit_context);

   REQUIRE(world.animations.size() == 3);
   CHECK(world.animations[0].name == "Animation0");
   CHECK(world.animations[1].name == "Animation1");
   CHECK(world.animations[2].name == "Animation2");

   REQUIRE(world.animation_groups[0].entries.size() == 3);
   CHECK(world.animation_groups[0].entries[0].animation == "Animation0");
   CHECK(world.animation_groups[0].entries[1].animation == "Animation1");
   CHECK(world.animation_groups[0].entries[2].animation == "Animation2");

   REQUIRE(world.animation_groups[1].entries.size() == 2);
   CHECK(world.animation_groups[1].entries[0].animation == "Animation1");
   CHECK(world.animation_groups[1].entries[1].animation == "Animation2");

   REQUIRE(world.animation_groups[2].entries.size() == 2);
   CHECK(world.animation_groups[2].entries[0].animation == "animation2");
   CHECK(world.animation_groups[2].entries[1].animation == "animation1");
}
}
