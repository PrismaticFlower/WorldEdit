#include "pch.h"

#include "edits/delete_sun_flare.hpp"

#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const world::world delete_sun_flare_test_world = {
   .effects =
      {
         .sun_flares =
            {
               pinned_vector_init{.max_size = 128, .initial_capacity = 128},
               std::initializer_list<world::sun_flare>{
                  world::sun_flare{
                     .num_flare_outs = {64, 64, 64, true},
                  },
                  world::sun_flare{
                     .num_flare_outs = {96, 96, 96, true},
                  },
                  world::sun_flare{
                     .num_flare_outs = {128, 128, 128, true},
                  },
               },
            },
      },
};

}

TEST_CASE("edits delete_sun_flare", "[Edits]")
{
   world::world world = delete_sun_flare_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_sun_flare(1);

   edit->apply(edit_context);

   REQUIRE(world.effects.sun_flares.size() == 2);
   CHECK(world.effects.sun_flares[0] ==
         delete_sun_flare_test_world.effects.sun_flares[0]);
   CHECK(world.effects.sun_flares[1] ==
         delete_sun_flare_test_world.effects.sun_flares[2]);

   edit->revert(edit_context);

   REQUIRE(world.effects.sun_flares.size() == 3);
   CHECK(world.effects.sun_flares == delete_sun_flare_test_world.effects.sun_flares);
}
}
