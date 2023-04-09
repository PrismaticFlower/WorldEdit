#include "pch.h"

#include "edits/delete_world_req_entry.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world delete_world_req_entry_test_world = {
   .name = "Test"s,

   .requirements =
      {
         {.file_type = "world", .entries = {"Test"}},
         {.file_type = "config", .entries = {"Test", "Best", "Quest"}},
         {.file_type = "terrain", .entries = {"Test"}},
      },
};

}

TEST_CASE("edits delete_world_req_entry", "[Edits]")
{
   world::world world = delete_world_req_entry_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_delete_world_req_entry(1, 1, world);

   action->apply(edit_context);

   REQUIRE(world.requirements.size() == 3);
   REQUIRE(world.requirements[0].entries.size() == 1);
   REQUIRE(world.requirements[1].entries.size() == 2);
   REQUIRE(world.requirements[2].entries.size() == 1);

   CHECK(world.requirements[1].entries[0] == "Test");
   CHECK(world.requirements[1].entries[1] == "Quest");

   action->revert(edit_context);

   REQUIRE(world.requirements == delete_world_req_entry_test_world.requirements);
}

}
