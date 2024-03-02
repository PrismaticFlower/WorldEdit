#include "pch.h"

#include "edits/set_value.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits set_memory_value", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_set_memory_value(&world.global_lights.env_map_texture, "starfield"s);

   edit->apply(edit_context);

   REQUIRE(world.global_lights.env_map_texture == "starfield");

   edit->revert(edit_context);

   REQUIRE(world.global_lights.env_map_texture ==
           test_world.global_lights.env_map_texture);
}

TEST_CASE("edits set_vector_value", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_vector_value(&world.paths[0].nodes, 0,
                                     &world::path::node::position,
                                     float3{1.0f, 1.0f, 1.0f});

   edit->apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{1.0f, 1.0f, 1.0f});

   edit->revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_vector_value alt constructor", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_vector_value(&world.sectors[0].points, 0, float2{8.0f, 8.0f});

   edit->apply(edit_context);

   REQUIRE(world.sectors[0].points[0] == float2{8.0f, 8.0f});

   edit->revert(edit_context);

   REQUIRE(world.sectors[0].points[0] == float2{0.0f, 0.0f});
}

TEST_CASE("edits set_multi_value2", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::portal{.width = 1.0f, .height = 2.0f};

   world::portal& portal = interaction_targets.creation_entity.get<world::portal>();

   auto edit = make_set_multi_value(&portal.width, 2.0f, &portal.height, 4.0f);

   edit->apply(edit_context);

   REQUIRE(portal.width == 2.0f);
   REQUIRE(portal.height == 4.0f);

   edit->revert(edit_context);

   REQUIRE(portal.width == 1.0f);
   REQUIRE(portal.height == 2.0f);
}

TEST_CASE("edits set_multi_value3", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::barrier{.size = {0.0f, 0.0f}};

   world::barrier& barrier =
      interaction_targets.creation_entity.get<world::barrier>();

   auto edit = make_set_multi_value(&barrier.rotation_angle, 2.0f,
                                    &barrier.position, float3{1.0f, 1.0f, 1.0f},
                                    &barrier.size, float2{2.0f, 2.0f});

   edit->apply(edit_context);

   REQUIRE(barrier.rotation_angle == 2.0f);
   REQUIRE(barrier.position == float3{1.0f, 1.0f, 1.0f});
   REQUIRE(barrier.size == float2{2.0f, 2.0f});

   edit->revert(edit_context);

   REQUIRE(barrier.rotation_angle == 0.0f);
   REQUIRE(barrier.position == float3{0.0f, 0.0f, 0.0f});
   REQUIRE(barrier.size == float2{0.0f, 0.0f});
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

   interaction_targets.creation_entity = world::object{};

   edit.apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::object>().rotation ==
           quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::object>().position ==
           float3{1.0f, 1.0f, 1.0f});
   REQUIRE(edit_context.euler_rotation == float3{2.0f, 2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::object>().rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::object>().position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_path_node_location", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   auto edit =
      make_set_creation_path_node_location(quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                           quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                           float3{1.0f, 1.0f, 1.0f},
                                           float3{0.0f, 0.0f, 0.0f},
                                           float3{2.0f, 2.0f, 2.0f},
                                           float3{0.0f, 0.0f, 0.0f});

   edit->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::path>().nodes[0].rotation ==
           quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::path>().nodes[0].position ==
           float3{1.0f, 1.0f, 1.0f});
   REQUIRE(edit_context.euler_rotation == float3{2.0f, 2.0f, 2.0f});

   edit->revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::path>().nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::path>().nodes[0].position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_region_metrics", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::region{};

   auto edit = make_set_creation_region_metrics(quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                                quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                                float3{1.0f, 1.0f, 1.0f},
                                                float3{0.0f, 0.0f, 0.0f},
                                                float3{2.0f, 2.0f, 2.0f},
                                                float3{0.0f, 0.0f, 0.0f});

   edit->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::region>().rotation ==
           quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::region>().position ==
           float3{1.0f, 1.0f, 1.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::region>().size ==
           float3{2.0f, 2.0f, 2.0f});

   edit->revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::region>().rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::region>().position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::region>().size ==
           float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_portal_size", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::portal{};

   auto edit = make_set_creation_portal_size(2.0f, 1.0f, 4.0f, 2.0f);

   edit->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::portal>().width == 2.0f);
   REQUIRE(interaction_targets.creation_entity.get<world::portal>().height == 4.0f);

   edit->revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::portal>().width == 1.0f);
   REQUIRE(interaction_targets.creation_entity.get<world::portal>().height == 2.0f);
}

TEST_CASE("edits set_creation_measurement_points", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::measurement{};

   auto edit = make_set_creation_measurement_points(float3{1.0f, 1.0f, 1.0f},
                                                    float3{0.0f, 0.0f, 0.0f},
                                                    float3{2.0f, 2.0f, 2.0f},
                                                    float3{0.5f, 0.5f, 0.5f});

   edit->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::measurement>().start ==
           float3{1.0f, 1.0f, 1.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::measurement>().end ==
           float3{2.0f, 2.0f, 2.0f});

   edit->revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::measurement>().start ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::measurement>().end ==
           float3{0.5f, 0.5f, 0.5f});
}

TEST_CASE("edits set_memory_value coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_set_memory_value(&world.global_lights.env_map_texture, "starfield"s);
   auto other_edit =
      make_set_memory_value(&world.global_lights.env_map_texture, "mountains"s);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(world.global_lights.env_map_texture == "mountains");

   edit->revert(edit_context);

   REQUIRE(world.global_lights.env_map_texture ==
           test_world.global_lights.env_map_texture);
}

TEST_CASE("edits set_vector_value coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_vector_value(&world.paths[0].nodes, 0,
                                     &world::path::node::position,
                                     float3{1.0f, 1.0f, 1.0f});
   auto other_edit = make_set_vector_value(&world.paths[0].nodes, 0,
                                           &world::path::node::position,
                                           float3{2.0f, 2.0f, 2.0f});

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{2.0f, 2.0f, 2.0f});

   edit->revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_vector_value alt constructor coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit =
      make_set_vector_value(&world.sectors[0].points, 0, float2{16.0f, 16.0f});
   auto other_edit =
      make_set_vector_value(&world.sectors[0].points, 0, float2{32.0f, 32.0f});

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(world.sectors[0].points[0] == float2{32.0f, 32.0f});

   edit->revert(edit_context);

   REQUIRE(world.sectors[0].points[0] == float2{0.0f, 0.0f});
}

TEST_CASE("edits set_multi_value2 coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::portal{.width = 1.0f, .height = 2.0f};

   world::portal& portal = interaction_targets.creation_entity.get<world::portal>();

   auto edit = make_set_multi_value(&portal.width, 2.0f, &portal.height, 4.0f);
   auto other_edit = make_set_multi_value(&portal.width, 8.0f, &portal.height, 16.0f);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(portal.width == 8.0f);
   REQUIRE(portal.height == 16.0f);

   edit->revert(edit_context);

   REQUIRE(portal.width == 1.0f);
   REQUIRE(portal.height == 2.0f);
}

TEST_CASE("edits set_multi_value3 coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::barrier{.size = {0.0f, 0.0f}};

   world::barrier& barrier =
      interaction_targets.creation_entity.get<world::barrier>();

   auto edit = make_set_multi_value(&barrier.rotation_angle, 1.0f,
                                    &barrier.position, float3{1.0f, 1.0f, 1.0f},
                                    &barrier.size, float2{2.0f, 2.0f});
   auto other_edit = make_set_multi_value(&barrier.rotation_angle, 2.0f,
                                          &barrier.position, float3{2.0f, 2.0f, 2.0f},
                                          &barrier.size, float2{4.0f, 4.0f});

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(barrier.rotation_angle == 2.0f);
   REQUIRE(barrier.position == float3{2.0f, 2.0f, 2.0f});
   REQUIRE(barrier.size == float2{4.0f, 4.0f});

   edit->revert(edit_context);

   REQUIRE(barrier.rotation_angle == 0.0f);
   REQUIRE(barrier.position == float3{0.0f, 0.0f, 0.0f});
   REQUIRE(barrier.size == float2{0.0f, 0.0f});
}

TEST_CASE("edits set_creation_location coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   set_creation_location<world::object> edit{quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                             quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                             float3{1.0f, 1.0f, 1.0f},
                                             float3{0.0f, 0.0f, 0.0f},
                                             float3{2.0f, 2.0f, 2.0f},
                                             float3{0.0f, 0.0f, 0.0f}};
   set_creation_location<world::object> other_edit{quaternion{0.0f, 0.0f, 1.0f, 0.0f},
                                                   quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                                   float3{2.0f, 2.0f, 2.0f},
                                                   float3{0.0f, 0.0f, 0.0f},
                                                   float3{4.0f, 4.0f, 4.0f},
                                                   float3{0.0f, 0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::object>().rotation ==
           quaternion{0.0f, 0.0f, 1.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::object>().position ==
           float3{2.0f, 2.0f, 2.0f});
   REQUIRE(edit_context.euler_rotation == float3{4.0f, 4.0f, 4.0f});

   edit.revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::object>().rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::object>().position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_path_node_location coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   auto edit =
      make_set_creation_path_node_location(quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                           quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                           float3{1.0f, 1.0f, 1.0f},
                                           float3{0.0f, 0.0f, 0.0f},
                                           float3{2.0f, 2.0f, 2.0f},
                                           float3{0.0f, 0.0f, 0.0f});
   auto other_edit =
      make_set_creation_path_node_location(quaternion{0.0f, 0.0f, 1.0f, 0.0f},
                                           quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                           float3{2.0f, 2.0f, 2.0f},
                                           float3{0.0f, 0.0f, 0.0f},
                                           float3{4.0f, 4.0f, 4.0f},
                                           float3{0.0f, 0.0f, 0.0f});

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::path>().nodes[0].rotation ==
           quaternion{0.0f, 0.0f, 1.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::path>().nodes[0].position ==
           float3{2.0f, 2.0f, 2.0f});
   REQUIRE(edit_context.euler_rotation == float3{4.0f, 4.0f, 4.0f});

   edit->revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::path>().nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::path>().nodes[0].position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_region_metrics coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::region{};

   auto edit = make_set_creation_region_metrics(quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                                quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                                float3{1.0f, 1.0f, 1.0f},
                                                float3{0.0f, 0.0f, 0.0f},
                                                float3{2.0f, 2.0f, 2.0f},
                                                float3{0.0f, 0.0f, 0.0f});
   auto other_edit =
      make_set_creation_region_metrics(quaternion{0.0f, 0.0f, 1.0f, 0.0f},
                                       quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                       float3{2.0f, 2.0f, 2.0f},
                                       float3{0.0f, 0.0f, 0.0f},
                                       float3{4.0f, 4.0f, 4.0f},
                                       float3{0.0f, 0.0f, 0.0f});

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::region>().rotation ==
           quaternion{0.0f, 0.0f, 1.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::region>().position ==
           float3{2.0f, 2.0f, 2.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::region>().size ==
           float3{4.0f, 4.0f, 4.0f});

   edit->revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::region>().rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::region>().position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::region>().size ==
           float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_portal_size coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::portal{};

   auto edit = make_set_creation_portal_size(2.0f, 1.0f, 4.0f, 2.0f);
   auto other_edit = make_set_creation_portal_size(8.0f, 2.0f, 16.0f, 4.0f);

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::portal>().width == 8.0f);
   REQUIRE(interaction_targets.creation_entity.get<world::portal>().height == 16.0f);

   edit->revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::portal>().width == 1.0f);
   REQUIRE(interaction_targets.creation_entity.get<world::portal>().height == 2.0f);
}

TEST_CASE("edits set_creation_measurement_points coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::measurement{};

   auto edit = make_set_creation_measurement_points(float3{1.0f, 1.0f, 1.0f},
                                                    float3{0.0f, 0.0f, 0.0f},
                                                    float3{2.0f, 2.0f, 2.0f},
                                                    float3{0.5f, 0.5f, 0.5f});
   auto other_edit =
      make_set_creation_measurement_points(float3{3.0f, 3.0f, 3.0f},
                                           float3{1.0f, 1.0f, 1.0f},
                                           float3{4.0f, 4.0f, 4.0f},
                                           float3{2.0f, 2.0f, 2.0f});

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);

   edit->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::measurement>().start ==
           float3{3.0f, 3.0f, 3.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::measurement>().end ==
           float3{4.0f, 4.0f, 4.0f});

   edit->revert(edit_context);

   REQUIRE(interaction_targets.creation_entity.get<world::measurement>().start ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(interaction_targets.creation_entity.get<world::measurement>().end ==
           float3{0.5f, 0.5f, 0.5f});
}
}
