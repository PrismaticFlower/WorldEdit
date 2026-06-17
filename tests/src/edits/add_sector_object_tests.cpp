#include "pch.h"

#include "edits/add_sector_object.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_sector_object tests", "[Edits]")
{
   we::world::world world = {
      .name = "Test"s,

      .layer_descriptions = {{.name = "[Base]"s}},
      .game_modes = {},
      .common_layers = {0},

      .objects =
         {
            world::entities_init,
            std::initializer_list{
               world::object{
                  .name = "test_object"s,
                  .layer = 0,
               },
               world::object{
                  .name = "such_an_object"s,
                  .layer = 0,
               },
            },
         },

      .sectors =
         {
            world::entities_init,
            std::initializer_list{
               world::sector{
                  .name = "test_sector"s,

                  .base = 0.0f,
                  .height = 0.0f,
                  .points = {{0.0f, 0.0f}, {0.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 0.0f}},
                  .objects = {0},
               },
            },
         },

   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_sector_object(&world.sectors[0].objects, 1);

   action->apply(edit_context);

   REQUIRE(world.sectors[0].objects.size() == 2);
   CHECK(world.sectors[0].objects[0] == 0);
   CHECK(world.sectors[0].objects[1] == 1);

   action->revert(edit_context);

   REQUIRE(world.sectors[0].objects.size() == 1);
   CHECK(world.sectors[0].objects[0] == 0);
}

}
