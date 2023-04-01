#include "pch.h"

#include "edits/add_game_mode.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world add_game_mode_test_world = {
   .name = "Test"s,

   .requirements =
      {
         {.file_type = "world"},
         {.file_type = "lvl",
          .entries = {"Test_conquest", "Test_ctf", "Test_hunt", "Test_assault"}},
      },

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

TEST_CASE("edits add_game_mode", "[Edits]")
{
   world::world world = add_game_mode_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_game_mode("race", world);

   action->apply(edit_context);

   REQUIRE(world.game_modes.size() == 6);
   CHECK(world.game_modes[0].name == "Common");
   CHECK(world.game_modes[1].name == "conquest");
   CHECK(world.game_modes[2].name == "ctf");
   CHECK(world.game_modes[3].name == "hunt");
   CHECK(world.game_modes[4].name == "assault");
   CHECK(world.game_modes[5].name == "race");

   REQUIRE(world.requirements.size() == 2);
   CHECK(world.requirements[1].file_type == "lvl");
   REQUIRE(world.requirements[1].entries.size() == 5);
   CHECK(world.requirements[1].entries[0] == "Test_conquest");
   CHECK(world.requirements[1].entries[1] == "Test_ctf");
   CHECK(world.requirements[1].entries[2] == "Test_hunt");
   CHECK(world.requirements[1].entries[3] == "Test_assault");
   CHECK(world.requirements[1].entries[4] == "Test_race");

   action->revert(edit_context);

   REQUIRE(world.game_modes == add_game_mode_test_world.game_modes);
   REQUIRE(world.requirements == add_game_mode_test_world.requirements);
}

TEST_CASE("edits add_game_mode empty req", "[Edits]")
{
   world::world world = add_game_mode_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world.requirements.pop_back();

   auto action = make_add_game_mode("race", world);

   action->apply(edit_context);

   REQUIRE(world.game_modes.size() == 6);
   CHECK(world.game_modes[0].name == "Common");
   CHECK(world.game_modes[1].name == "conquest");
   CHECK(world.game_modes[2].name == "ctf");
   CHECK(world.game_modes[3].name == "hunt");
   CHECK(world.game_modes[4].name == "assault");
   CHECK(world.game_modes[5].name == "race");

   REQUIRE(world.requirements.size() == 2);
   CHECK(world.requirements[1].file_type == "lvl");
   REQUIRE(world.requirements[1].entries.size() == 1);
   CHECK(world.requirements[1].entries[0] == "Test_race");

   action->revert(edit_context);

   REQUIRE(world.game_modes == add_game_mode_test_world.game_modes);

   REQUIRE(world.requirements.size() == 1);
   CHECK(world.requirements[0] == add_game_mode_test_world.requirements[0]);
}

}
