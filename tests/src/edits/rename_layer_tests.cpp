#include "pch.h"

#include "edits/rename_layer.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const world::world rename_layer_test_world = {
   .name = "test",

   .requirements =
      {
         {
            .file_type = "world",

            .entries = {"test_common"},
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
            .requirements =
               {
                  {
                     .file_type = "world",

                     .entries = {"test_conquest"},
                  },
               },
         },

         {
            .name = "ctf",
            .layers = {3},
            .requirements =
               {
                  {
                     .file_type = "world",

                     .entries = {"test_ctf"},
                  },
               },
         },
      },
};

}

TEST_CASE("edits rename_layer world ref", "[Edits]")
{
   world::world world = rename_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_layer(1, "design", world);

   edit->apply(edit_context);

   CHECK(world.layer_descriptions[1].name == "design");
   CHECK(world.requirements[0].entries[0] == "test_design");

   REQUIRE(world.deleted_layers.size() == 1);
   CHECK(world.deleted_layers[0] == "common");

   edit->revert(edit_context);

   CHECK(world.layer_descriptions[1].name == "common");
   CHECK(world.requirements[0].entries[0] == "test_common");

   CHECK(world.deleted_layers.size() == 0);

   CHECK(world.layer_descriptions == rename_layer_test_world.layer_descriptions);
   CHECK(world.game_modes == rename_layer_test_world.game_modes);
   CHECK(world.requirements == rename_layer_test_world.requirements);
}

TEST_CASE("edits rename_layer game mode ref", "[Edits]")
{
   world::world world = rename_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_layer(2, "tdm", world);

   edit->apply(edit_context);

   CHECK(world.layer_descriptions[2].name == "tdm");
   CHECK(world.game_modes[0].requirements[0].entries[0] == "test_tdm");

   REQUIRE(world.deleted_layers.size() == 1);
   CHECK(world.deleted_layers[0] == "conquest");

   edit->revert(edit_context);

   CHECK(world.layer_descriptions[2].name == "conquest");
   CHECK(world.game_modes[0].requirements[0].entries[0] == "test_conquest");

   CHECK(world.deleted_layers.size() == 0);

   CHECK(world.layer_descriptions == rename_layer_test_world.layer_descriptions);
   CHECK(world.game_modes == rename_layer_test_world.game_modes);
   CHECK(world.requirements == rename_layer_test_world.requirements);
}

TEST_CASE("edits rename_layer no ref", "[Edits]")
{
   world::world world = rename_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_layer(4, "soundscape", world);

   edit->apply(edit_context);

   CHECK(world.layer_descriptions[4].name == "soundscape");

   REQUIRE(world.deleted_layers.size() == 1);
   CHECK(world.deleted_layers[0] == "sound");

   edit->revert(edit_context);

   CHECK(world.layer_descriptions[4].name == "sound");

   CHECK(world.deleted_layers.size() == 0);

   CHECK(world.layer_descriptions == rename_layer_test_world.layer_descriptions);
   CHECK(world.game_modes == rename_layer_test_world.game_modes);
   CHECK(world.requirements == rename_layer_test_world.requirements);
}

TEST_CASE("edits rename_layer world ref coalesce", "[Edits]")
{
   world::world world = rename_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_layer(1, "desig", world);
   auto other_edit = make_rename_layer(1, "design", world);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(world.layer_descriptions[1].name == "design");
   CHECK(world.requirements[0].entries[0] == "test_design");

   REQUIRE(world.deleted_layers.size() == 1);
   CHECK(world.deleted_layers[0] == "common");

   edit->revert(edit_context);

   CHECK(world.layer_descriptions[1].name == "common");
   CHECK(world.requirements[0].entries[0] == "test_common");

   CHECK(world.deleted_layers.size() == 0);

   CHECK(world.layer_descriptions == rename_layer_test_world.layer_descriptions);
   CHECK(world.game_modes == rename_layer_test_world.game_modes);
   CHECK(world.requirements == rename_layer_test_world.requirements);
}

TEST_CASE("edits rename_layer game mode ref coalesce", "[Edits]")
{
   world::world world = rename_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_layer(2, "td", world);
   auto other_edit = make_rename_layer(2, "tdm", world);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(world.layer_descriptions[2].name == "tdm");
   CHECK(world.game_modes[0].requirements[0].entries[0] == "test_tdm");

   REQUIRE(world.deleted_layers.size() == 1);
   CHECK(world.deleted_layers[0] == "conquest");

   edit->revert(edit_context);

   CHECK(world.layer_descriptions[2].name == "conquest");
   CHECK(world.game_modes[0].requirements[0].entries[0] == "test_conquest");

   CHECK(world.deleted_layers.size() == 0);

   CHECK(world.layer_descriptions == rename_layer_test_world.layer_descriptions);
   CHECK(world.game_modes == rename_layer_test_world.game_modes);
   CHECK(world.requirements == rename_layer_test_world.requirements);
}

TEST_CASE("edits rename_layer no ref coalesce", "[Edits]")
{
   world::world world = rename_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_layer(4, "soundscap", world);
   auto other_edit = make_rename_layer(4, "soundscape", world);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(world.layer_descriptions[4].name == "soundscape");

   REQUIRE(world.deleted_layers.size() == 1);
   CHECK(world.deleted_layers[0] == "sound");

   edit->revert(edit_context);

   CHECK(world.layer_descriptions[4].name == "sound");

   CHECK(world.deleted_layers.size() == 0);

   CHECK(world.layer_descriptions == rename_layer_test_world.layer_descriptions);
   CHECK(world.game_modes == rename_layer_test_world.game_modes);
   CHECK(world.requirements == rename_layer_test_world.requirements);
}

TEST_CASE("edits rename_layer world ref no coalesce", "[Edits]")
{
   world::world world = rename_layer_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_rename_layer(1, "desig", world);
   auto other_edit = make_rename_layer(2, "design", world);

   REQUIRE(not edit->is_coalescable(*other_edit));
}

}
