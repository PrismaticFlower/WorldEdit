#include "pch.h"

#include "edits/game_mode_unlink_layer.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world game_mode_unlink_layer_test_world = {
   .name = "Test"s,

   .layer_descriptions =
      {
         {.name = "[Base]"},
         {.name = "A"},
         {.name = "B"},
         {.name = "C"},
      },
   .game_modes =
      {
         {.name = "Common", .layers = {0}},
         {.name = "conquest",
          .layers = {1, 2, 3},
          .requirements = {{
             .file_type = "world",
             .entries =
                {
                   "Test_A",
                   "Test_B",
                   "Test_C",
                },
          }}},
      },
};

const we::world::world game_mode_unlink_layer_empty_req_test_world = {
   .name = "Test"s,

   .layer_descriptions =
      {
         {.name = "[Base]"},
         {.name = "A"},
         {.name = "B"},
         {.name = "C"},
      },
   .game_modes =
      {
         {.name = "Common", .layers = {0}},
         {.name = "conquest", .layers = {1, 2, 3}},
      },
};

}

TEST_CASE("edits game_mode_unlink_layer", "[Edits]")
{
   world::world world = game_mode_unlink_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_game_mode_unlink_layer(1, 1, world);

   action->apply(edit_context);

   REQUIRE(world.game_modes[1].layers.size() == 2);
   CHECK(world.game_modes[1].layers[0] == 1);
   CHECK(world.game_modes[1].layers[1] == 3);

   REQUIRE(world.game_modes[1].requirements.size() == 1);
   CHECK(world.game_modes[1].requirements[0].file_type == "world");
   REQUIRE(world.game_modes[1].requirements[0].entries.size() == 2);
   CHECK(world.game_modes[1].requirements[0].entries[0] == "Test_A");
   CHECK(world.game_modes[1].requirements[0].entries[1] == "Test_C");

   action->revert(edit_context);

   REQUIRE(world.game_modes == game_mode_unlink_layer_test_world.game_modes);
}

TEST_CASE("edits game_mode_unlink_layer empty req", "[Edits]")
{
   world::world world = game_mode_unlink_layer_empty_req_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_game_mode_unlink_layer(1, 1, world);

   action->apply(edit_context);

   REQUIRE(world.game_modes[1].layers.size() == 2);
   CHECK(world.game_modes[1].layers[0] == 1);
   CHECK(world.game_modes[1].layers[1] == 3);

   REQUIRE(world.game_modes[1].requirements.empty());

   action->revert(edit_context);

   REQUIRE(world.game_modes == game_mode_unlink_layer_empty_req_test_world.game_modes);
}

TEST_CASE("edits game_mode_unlink_layer common", "[Edits]")
{
   world::world world = game_mode_unlink_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_game_mode_unlink_layer(0, 0, world);

   action->apply(edit_context);

   REQUIRE(world.game_modes[0].layers.empty());
   REQUIRE(world.game_modes[0].requirements.empty());

   action->revert(edit_context);

   REQUIRE(world.game_modes == game_mode_unlink_layer_test_world.game_modes);
}

}
