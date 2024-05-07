#include "pch.h"

#include "edits/delete_animation_key.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world
   test_world_delete_animation_key =
      {.animations =
          {
             world::entities_init,
             std::initializer_list{
                world::animation{
                   .name = "Animation",
                   .runtime = 8.0f,
                   .position_keys =
                      {
                         world::position_key{.time = 0.0f, .position = {0.0f, 0.0f, 0.0f}},
                         world::position_key{.time = 5.0f, .position = {4.0f, 8.0f, 8.0f}},
                         world::position_key{.time = 7.0f, .position = {8.0f, 8.0f, 8.0f}},
                      },
                },
             },
          }};
}

TEST_CASE("edits delete_animation_key", "[Edits]")
{
   world::world world = test_world_delete_animation_key;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   const float2 point = {1e10f, 0.0f};
   const world::position_key key{.time = 4.0f, .position = {4.0f, 4.0f, 4.0f}};

   auto edit = make_delete_animation_key(&world.animations[0].position_keys, 1);

   edit->apply(edit_context);

   REQUIRE(world.animations[0].position_keys.size() == 2);
   CHECK(world.animations[0].position_keys[0].time == 0.0f);
   CHECK(world.animations[0].position_keys[1].time == 7.0f);

   edit->revert(edit_context);

   REQUIRE(world.animations[0].position_keys.size() == 3);
   CHECK(world.animations[0].position_keys[0].time == 0.0f);
   CHECK(world.animations[0].position_keys[1].time == 5.0f);
   CHECK(world.animations[0].position_keys[2].time == 7.0f);
}
}
