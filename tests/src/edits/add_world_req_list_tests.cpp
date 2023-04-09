#include "pch.h"

#include "edits/add_world_req_list.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world add_world_req_list_test_world = {
   .name = "Test"s,

   .requirements =
      {
         {.file_type = "world"},
         {.file_type = "config"},
         {.file_type = "terrain"},
      },
};

}

TEST_CASE("edits add_world_req_list", "[Edits]")
{
   world::world world = add_world_req_list_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_add_world_req_list("script");

   action->apply(edit_context);

   REQUIRE(world.requirements.size() == 4);
   CHECK(world.requirements[0].file_type == "world");
   CHECK(world.requirements[1].file_type == "config");
   CHECK(world.requirements[2].file_type == "terrain");
   CHECK(world.requirements[3].file_type == "script");

   action->revert(edit_context);

   REQUIRE(world.requirements == add_world_req_list_test_world.requirements);
}

}
