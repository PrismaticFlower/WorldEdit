#include "pch.h"

#include "edits/rename_game_mode.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const world::world rename_game_mode_test_world = {
   .name = "test",

   .requirements =
      {
         {
            .file_type = "lvl",

            .entries = {"test_conquest"},
         },
      },

   .layer_descriptions =
      {
         {"[Base]"},
         {"common"},
         {"conquest"},
         {"ctf"},
         {"sound"},
      },

   .game_modes =
      {
         {
            .name = "conquest",
            .layers = {2},
         },

         {
            .name = "ctf",
            .layers = {3},
         },
      },
};

}

TEST_CASE("edits rename_game_mode world ref", "[Edits]")
{
   world::world world = rename_game_mode_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_game_mode(0, "tdm", world);

   edit->apply(edit_context);

   CHECK(world.game_modes[0].name == "tdm");
   CHECK(world.requirements[0].entries[0] == "test_tdm");

   REQUIRE(world.deleted_game_modes.size() == 1);
   CHECK(world.deleted_game_modes[0] == "conquest");

   edit->revert(edit_context);

   CHECK(world.game_modes[0].name == "conquest");
   CHECK(world.requirements[0].entries[0] == "test_conquest");

   CHECK(world.deleted_game_modes.size() == 0);

   CHECK(world.game_modes == rename_game_mode_test_world.game_modes);
   CHECK(world.requirements == rename_game_mode_test_world.requirements);
}

TEST_CASE("edits rename_game_mode no ref", "[Edits]")
{
   world::world world = rename_game_mode_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_game_mode(1, "1flag", world);

   edit->apply(edit_context);

   CHECK(world.game_modes[1].name == "1flag");

   REQUIRE(world.deleted_game_modes.size() == 1);
   CHECK(world.deleted_game_modes[0] == "ctf");

   edit->revert(edit_context);

   CHECK(world.game_modes[1].name == "ctf");

   CHECK(world.deleted_game_modes.size() == 0);

   CHECK(world.game_modes == rename_game_mode_test_world.game_modes);
   CHECK(world.requirements == rename_game_mode_test_world.requirements);
}

TEST_CASE("edits rename_game_mode world ref coalesce", "[Edits]")
{
   world::world world = rename_game_mode_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_game_mode(0, "eli", world);
   auto other_edit = make_rename_game_mode(0, "tdm", world);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(world.game_modes[0].name == "tdm");
   CHECK(world.requirements[0].entries[0] == "test_tdm");

   REQUIRE(world.deleted_game_modes.size() == 1);
   CHECK(world.deleted_game_modes[0] == "conquest");

   edit->revert(edit_context);

   CHECK(world.game_modes[0].name == "conquest");
   CHECK(world.requirements[0].entries[0] == "test_conquest");

   CHECK(world.deleted_game_modes.size() == 0);

   CHECK(world.game_modes == rename_game_mode_test_world.game_modes);
   CHECK(world.requirements == rename_game_mode_test_world.requirements);
}

TEST_CASE("edits rename_game_mode no ref coalesce", "[Edits]")
{
   world::world world = rename_game_mode_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_game_mode(1, "hunt", world);
   auto other_edit = make_rename_game_mode(1, "1flag", world);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(world.game_modes[1].name == "1flag");

   REQUIRE(world.deleted_game_modes.size() == 1);
   CHECK(world.deleted_game_modes[0] == "ctf");

   edit->revert(edit_context);

   CHECK(world.game_modes[1].name == "ctf");

   CHECK(world.deleted_game_modes.size() == 0);

   CHECK(world.game_modes == rename_game_mode_test_world.game_modes);
   CHECK(world.requirements == rename_game_mode_test_world.requirements);
}

TEST_CASE("edits rename_game_mode world ref no coalesce", "[Edits]")
{
   world::world world = rename_game_mode_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_game_mode(0, "1flag", world);
   auto other_edit = make_rename_game_mode(1, "ctf", world);

   REQUIRE(not edit->is_coalescable(*other_edit));
}

}
