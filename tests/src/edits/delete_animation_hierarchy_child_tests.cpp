#include "pch.h"

#include "edits/delete_animation_hierarchy_child.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world test_world_delete_animation_hierarchy_child = {
   .animation_hierarchies = {
      world::entities_init,
      std::initializer_list{
         world::animation_hierarchy{
            .root_object = world::object_optional_link{"ship"},
            .objects =
               {
                  0,
                  1,
                  2,
               },
         },
      },
   }};

const we::world::world test_world_delete_animation_hierarchy_child_broken_links = {
   .animation_hierarchies = {
      world::entities_init,
      std::initializer_list{
         world::animation_hierarchy{
            .root_object = world::object_optional_link{"ship"},
            .objects_broken_links =
               {
                  {"Gun0"},
                  {"Gun1"},
                  {"Gun2"},
               },
         },
      },
   }};
}

TEST_CASE("edits delete_animation_hierarchy_child", "[Edits]")
{
   world::world world = test_world_delete_animation_hierarchy_child;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_delete_animation_hierarchy_child(&world.animation_hierarchies[0].objects, 1);

   edit->apply(edit_context);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 2);
   CHECK(world.animation_hierarchies[0].objects[0] == 0);
   CHECK(world.animation_hierarchies[0].objects[1] == 2);

   edit->revert(edit_context);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 3);
   CHECK(world.animation_hierarchies[0].objects[0] == 0);
   CHECK(world.animation_hierarchies[0].objects[1] == 1);
   CHECK(world.animation_hierarchies[0].objects[2] == 2);
}

TEST_CASE("edits delete_animation_hierarchy_child (broken links)", "[Edits]")
{
   world::world world = test_world_delete_animation_hierarchy_child_broken_links;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_delete_animation_hierarchy_child(&world.animation_hierarchies[0].objects_broken_links,
                                            1);

   edit->apply(edit_context);

   REQUIRE(world.animation_hierarchies[0].objects_broken_links.size() == 2);
   CHECK(world.animation_hierarchies[0].objects_broken_links[0] == "Gun0");
   CHECK(world.animation_hierarchies[0].objects_broken_links[1] == "Gun2");

   edit->revert(edit_context);

   REQUIRE(world.animation_hierarchies[0].objects_broken_links.size() == 3);
   CHECK(world.animation_hierarchies[0].objects_broken_links[0] == "Gun0");
   CHECK(world.animation_hierarchies[0].objects_broken_links[1] == "Gun1");
   CHECK(world.animation_hierarchies[0].objects_broken_links[2] == "Gun2");
}
}
