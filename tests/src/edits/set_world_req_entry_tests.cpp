#include "pch.h"

#include "edits/set_world_req_entry.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const world::world set_req_entry_test_world = {
   .requirements =
      {
         {
            .file_type = "model",

            .entries =
               {
                  "sky",
                  "ground",
               },
         },

         {
            .file_type = "texture",

            .entries =
               {
                  "stars",
                  "dirt",
               },
         },
      },

};

}

TEST_CASE("edits set_world_req_entry", "[Edits]")
{
   world::world world = set_req_entry_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_world_req_entry(1, 1, "grass");

   edit->apply(edit_context);

   CHECK(world.requirements[1].entries[1] == "grass");

   edit->revert(edit_context);

   CHECK(world.requirements[1].entries[1] == "dirt");

   CHECK(world.requirements == set_req_entry_test_world.requirements);
}

TEST_CASE("edits set_world_req_entry coalesce", "[Edits]")
{
   world::world world = set_req_entry_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_world_req_entry(1, 1, "glass");
   auto other_edit = make_set_world_req_entry(1, 1, "grass");

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   CHECK(world.requirements[1].entries[1] == "grass");

   edit->revert(edit_context);

   CHECK(world.requirements[1].entries[1] == "dirt");

   CHECK(world.requirements == set_req_entry_test_world.requirements);
}

TEST_CASE("edits set_world_req_entry no coalesce", "[Edits]")
{
   world::world world = set_req_entry_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_world_req_entry(0, 0, "glass");
   auto other_edit = make_set_world_req_entry(1, 0, "grass");

   REQUIRE(not edit->is_coalescable(*other_edit));
}

}
