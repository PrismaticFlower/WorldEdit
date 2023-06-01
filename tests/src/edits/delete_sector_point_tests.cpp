#include "pch.h"

#include "edits/delete_sector_point.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits delete_sector_point", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit_0 = make_delete_sector_point(world.sectors[0].id, 2, world);

   edit_0->apply(edit_context);

   REQUIRE(world.sectors.size() == 1);
   REQUIRE(world.sectors[0].points.size() == 3);
   CHECK(world.sectors[0].points[0] == float2{0.0f, 0.0f});
   CHECK(world.sectors[0].points[1] == float2{0.0f, 10.0f});
   CHECK(world.sectors[0].points[2] == float2{10.0f, 0.0f});

   auto edit_1 = make_delete_sector_point(world.sectors[0].id, 1, world);

   edit_1->apply(edit_context);

   REQUIRE(world.sectors.size() == 1);
   REQUIRE(world.sectors[0].points.size() == 2);
   CHECK(world.sectors[0].points[0] == float2{0.0f, 0.0f});
   CHECK(world.sectors[0].points[1] == float2{10.0f, 0.0f});

   auto edit_2 = make_delete_sector_point(world.sectors[0].id, 0, world);

   edit_2->apply(edit_context);

   REQUIRE(world.sectors.size() == 1);
   REQUIRE(world.sectors[0].points.size() == 1);
   CHECK(world.sectors[0].points[0] == float2{10.0f, 0.0f});

   auto edit_3 = make_delete_sector_point(world.sectors[0].id, 0, world);

   edit_3->apply(edit_context);

   REQUIRE(world.sectors.size() == 1);
   REQUIRE(world.sectors[0].points.empty());

   edit_3->revert(edit_context);
   edit_2->revert(edit_context);
   edit_1->revert(edit_context);
   edit_0->revert(edit_context);

   REQUIRE(world.sectors.size() == 1);
   CHECK(world.sectors[0] == test_world.sectors[0]);
}

}
