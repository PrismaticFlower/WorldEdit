#include "pch.h"

#include "edits/add_world_req_entry.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world add_world_req_entry_test_world = {
   .name = "Test"s,

   .requirements =
      {
         {.file_type = "world", .entries = {"Test"}},
         {.file_type = "config", .entries = {"Test"}},
         {.file_type = "terrain", .entries = {"Test"}},
      },
};

}

TEST_CASE("edits add_world_req_entry", "[Edits]")
{
   world::world world = add_world_req_entry_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_world_req_entry(1, "AmazingConfig");

   action->apply(edit_context);

   REQUIRE(world.requirements.size() == 3);
   REQUIRE(world.requirements[0].entries.size() == 1);
   REQUIRE(world.requirements[1].entries.size() == 2);
   REQUIRE(world.requirements[2].entries.size() == 1);

   CHECK(world.requirements[1].entries[0] == "Test");
   CHECK(world.requirements[1].entries[1] == "AmazingConfig");

   action->revert(edit_context);

   REQUIRE(world.requirements == add_world_req_entry_test_world.requirements);
}

}
