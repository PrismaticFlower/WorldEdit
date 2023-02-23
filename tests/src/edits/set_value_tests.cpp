#include "pch.h"

#include "edits/set_value.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits set_creation_value", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   set_creation_value edit{&world::object::layer, 1, 0};

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 1);

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
}

TEST_CASE("edits set_creation_value_with_meta", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   set_creation_value_with_meta edit{&world::object::layer,
                                     1,
                                     0,
                                     &world::edit_context::euler_rotation,
                                     {1.0f, 1.0f, 1.0f},
                                     {0.0f, 0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 1);
   REQUIRE(edit_context.euler_rotation == float3{1.0f, 1.0f, 1.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_location", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   set_creation_location<world::object> edit{quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                             quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                             float3{1.0f, 1.0f, 1.0f},
                                             float3{0.0f, 0.0f, 0.0f},
                                             float3{2.0f, 2.0f, 2.0f},
                                             float3{0.0f, 0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).rotation ==
           quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).position ==
           float3{1.0f, 1.0f, 1.0f});
   REQUIRE(edit_context.euler_rotation == float3{2.0f, 2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

}
