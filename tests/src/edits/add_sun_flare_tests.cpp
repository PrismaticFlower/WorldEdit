#include "pch.h"

#include "edits/add_sun_flare.hpp"

#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_sun_flare", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = edits::make_add_sun_flare(world::sun_flare{});

   edit->apply(edit_context);

   REQUIRE(world.effects.sun_flares.size() == 1);
   CHECK(world.effects.sun_flares[0] == world::sun_flare{});

   edit->revert(edit_context);

   REQUIRE(world.effects.sun_flares.empty());
}

}
