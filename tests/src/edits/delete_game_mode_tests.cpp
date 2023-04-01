#include "pch.h"

#include "edits/delete_game_mode.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world game_mode_delete_test_world = {
   .name = "Test"s,

   .requirements = {{.file_type = "lvl",
                     .entries = {"Test_conquest", "Test_ctf", "Test_hunt", "Test_assault"}}},

   .layer_descriptions =
      {
         {.name = "[Base]"},
      },
   .game_modes =
      {
         {.name = "Common"},
         {.name = "conquest"},
         {.name = "ctf"},
         {.name = "hunt"},
         {.name = "assault"},
      },
};

}

TEST_CASE("edits delete_game_mode", "[Edits]")
{
   world::world world = game_mode_delete_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_delete_game_mode(2, world);

   action->apply(edit_context);

   REQUIRE(world.requirements[0].entries.size() == 3);
   CHECK(world.requirements[0].entries[0] == "Test_conquest");
   CHECK(world.requirements[0].entries[1] == "Test_hunt");
   CHECK(world.requirements[0].entries[2] == "Test_assault");

   REQUIRE(world.game_modes.size() == 4);
   CHECK(world.game_modes[0].name == "Common");
   CHECK(world.game_modes[1].name == "conquest");
   CHECK(world.game_modes[2].name == "hunt");
   CHECK(world.game_modes[3].name == "assault");

   action->revert(edit_context);

   CHECK(world.requirements == game_mode_delete_test_world.requirements);
   CHECK(world.game_modes == game_mode_delete_test_world.game_modes);
}

}
