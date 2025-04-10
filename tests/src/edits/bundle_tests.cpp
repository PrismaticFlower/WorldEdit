#include "pch.h"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits bundle", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   bundle_vector bundle;

   bundle.push_back(make_set_value(&world.objects[0].team, 4));
   bundle.push_back(make_set_value(&world.objects[0].team, 8));

   auto edit = make_bundle(std::move(bundle));

   edit->apply(edit_context);

   REQUIRE(world.objects[0].team == 8);

   edit->revert(edit_context);

   REQUIRE(world.objects[0].team == 0);
}

TEST_CASE("edits bundle coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   bundle_vector bundle;

   bundle.push_back(make_set_value(&world.objects[0].team, 4));
   bundle.push_back(make_set_value(&world.objects[0].layer, int8{8}));

   auto edit = make_bundle(std::move(bundle));

   bundle_vector other_bundle;

   other_bundle.push_back(make_set_value(&world.objects[0].team, 8));
   other_bundle.push_back(make_set_value(&world.objects[0].layer, int8{16}));

   auto other_edit = make_bundle(std::move(other_bundle));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(world.objects[0].team == 8);
   REQUIRE(world.objects[0].layer == 16);

   edit->revert(edit_context);

   REQUIRE(world.objects[0].team == 0);
   REQUIRE(world.objects[0].layer == 0);
}

}
