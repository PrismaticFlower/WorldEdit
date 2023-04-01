#include "pch.h"

#include "edits/game_mode_link_layer.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world game_mode_link_layer_test_world = {
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
         {.name = "Common"},
         {.name = "conquest", .requirements = {{.file_type = "world"}}},
      },
};

const we::world::world game_mode_link_layer_empty_req_test_world = {
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
         {.name = "Common"},
         {.name = "conquest"},
      },
};

}

TEST_CASE("edits game_mode_link_layer", "[Edits]")
{
   world::world world = game_mode_link_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_game_mode_link_layer(1, 1, world);

   action->apply(edit_context);

   REQUIRE(world.game_modes[1].layers.size() == 1);
   CHECK(world.game_modes[1].layers[0] == 1);

   REQUIRE(world.game_modes[1].requirements.size() == 1);
   REQUIRE(world.game_modes[1].requirements[0].entries.size() == 1);
   CHECK(world.game_modes[1].requirements[0].file_type == "world");
   CHECK(world.game_modes[1].requirements[0].entries[0] == "Test_A");

   action->revert(edit_context);

   REQUIRE(world.game_modes == game_mode_link_layer_test_world.game_modes);
}

TEST_CASE("edits game_mode_link_layer empty req", "[Edits]")
{
   world::world world = game_mode_link_layer_empty_req_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_game_mode_link_layer(1, 1, world);

   action->apply(edit_context);

   REQUIRE(world.game_modes[1].layers.size() == 1);
   CHECK(world.game_modes[1].layers[0] == 1);

   REQUIRE(world.game_modes[1].requirements.size() == 1);
   REQUIRE(world.game_modes[1].requirements[0].entries.size() == 1);
   CHECK(world.game_modes[1].requirements[0].file_type == "world");
   CHECK(world.game_modes[1].requirements[0].entries[0] == "Test_A");

   action->revert(edit_context);

   REQUIRE(world.game_modes == game_mode_link_layer_empty_req_test_world.game_modes);
}

TEST_CASE("edits game_mode_link_layer common", "[Edits]")
{
   world::world world = game_mode_link_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_game_mode_link_layer(0, 1, world);

   action->apply(edit_context);

   REQUIRE(world.game_modes[0].layers.size() == 1);
   CHECK(world.game_modes[0].layers[0] == 1);

   REQUIRE(world.game_modes[0].requirements.empty());

   action->revert(edit_context);

   REQUIRE(world.game_modes == game_mode_link_layer_test_world.game_modes);
}

}
