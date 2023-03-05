#include "pch.h"

#include "edits/set_value.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits set_value", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   set_value edit{world.objects[0].id, &world::object::layer, 1, 0};

   edit.apply(edit_context);

   REQUIRE(world.objects[0].layer == 1);

   edit.revert(edit_context);

   REQUIRE(world.objects[0].layer == 0);
}

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

   interaction_targets.creation_entity = world::object{};

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

TEST_CASE("edits set_creation_path_node_value", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   set_creation_path_node_value edit{&world::path::node::rotation,
                                     {-1.0f, 0.0f, 0.0f, 0.0f},
                                     {1.0f, 0.0f, 0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{-1.0f, 0.0f, 0.0f, 0.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_path_node_location", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   set_creation_path_node_location edit{quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                        quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                        float3{1.0f, 1.0f, 1.0f},
                                        float3{0.0f, 0.0f, 0.0f},
                                        float3{2.0f, 2.0f, 2.0f},
                                        float3{0.0f, 0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].position ==
           float3{1.0f, 1.0f, 1.0f});
   REQUIRE(edit_context.euler_rotation == float3{2.0f, 2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_region_metrics", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::region{};

   set_creation_region_metrics edit{quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                    quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                    float3{1.0f, 1.0f, 1.0f},
                                    float3{0.0f, 0.0f, 0.0f},
                                    float3{2.0f, 2.0f, 2.0f},
                                    float3{0.0f, 0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).rotation ==
           quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).position ==
           float3{1.0f, 1.0f, 1.0f});
   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).size ==
           float3{2.0f, 2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).size ==
           float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_sector_point", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::sector{.points = {{0.0f, 0.0f}}};

   set_creation_sector_point edit{float2{1.0f, 1.0f}, float2{0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::sector>(*interaction_targets.creation_entity).points[0] ==
           float2{1.0f, 1.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::sector>(*interaction_targets.creation_entity).points[0] ==
           float2{0.0f, 0.0f});
}

TEST_CASE("edits set_creation_portal_size", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::portal{};

   set_creation_portal_size edit{2.0f, 1.0f, 4.0f, 2.0f};

   edit.apply(edit_context);

   REQUIRE(std::get<world::portal>(*interaction_targets.creation_entity).width == 2.0f);
   REQUIRE(std::get<world::portal>(*interaction_targets.creation_entity).height == 4.0f);

   edit.revert(edit_context);

   REQUIRE(std::get<world::portal>(*interaction_targets.creation_entity).width == 1.0f);
   REQUIRE(std::get<world::portal>(*interaction_targets.creation_entity).height == 2.0f);
}

TEST_CASE("edits set_creation_barrier_metrics", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::barrier{};

   set_creation_barrier_metrics edit{2.0f,
                                     0.0f,
                                     float2{1.0f, 1.0f},
                                     float2{0.0f, 0.0f},
                                     float2{2.0f, 2.0f},
                                     float2{0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).rotation_angle ==
           2.0f);
   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).position ==
           float2{1.0f, 1.0f});
   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).size ==
           float2{2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).rotation_angle ==
           0.0f);
   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).position ==
           float2{0.0f, 0.0f});
   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).size ==
           float2{0.0f, 0.0f});
}

TEST_CASE("edits set_value coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   set_value edit{world.objects[0].id, &world::object::layer, 1, 0};
   set_value other_edit{world.objects[0].id, &world::object::layer, 2, 0};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(world.objects[0].layer == 2);

   edit.revert(edit_context);

   REQUIRE(world.objects[0].layer == 0);
}

TEST_CASE("edits set_creation_value coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   set_creation_value edit{&world::object::layer, 1, 0};
   set_creation_value other_edit{&world::object::layer, 2, 0};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 2);

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
}

TEST_CASE("edits set_creation_value_with_meta coalesce", "[Edits]")
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
   set_creation_value_with_meta other_edit{&world::object::layer,
                                           2,
                                           0,
                                           &world::edit_context::euler_rotation,
                                           {2.0f, 2.0f, 2.0f},
                                           {0.0f, 0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 2);
   REQUIRE(edit_context.euler_rotation == float3{2.0f, 2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
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

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).rotation ==
           quaternion{0.0f, 0.0f, 1.0f, 0.0f});
   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).position ==
           float3{2.0f, 2.0f, 2.0f});
   REQUIRE(edit_context.euler_rotation == float3{4.0f, 4.0f, 4.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_path_node_value coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   set_creation_path_node_value edit{&world::path::node::rotation,
                                     {-1.0f, 0.0f, 0.0f, 0.0f},
                                     {1.0f, 0.0f, 0.0f, 0.0f}};
   set_creation_path_node_value other_edit{&world::path::node::rotation,
                                           {0.0f, 1.0f, 0.0f, 0.0f},
                                           {1.0f, 0.0f, 0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{0.0f, 1.0f, 0.0f, 0.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_path_node_location coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   set_creation_path_node_location edit{quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                        quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                        float3{1.0f, 1.0f, 1.0f},
                                        float3{0.0f, 0.0f, 0.0f},
                                        float3{2.0f, 2.0f, 2.0f},
                                        float3{0.0f, 0.0f, 0.0f}};
   set_creation_path_node_location other_edit{quaternion{0.0f, 0.0f, 1.0f, 0.0f},
                                              quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                              float3{2.0f, 2.0f, 2.0f},
                                              float3{0.0f, 0.0f, 0.0f},
                                              float3{4.0f, 4.0f, 4.0f},
                                              float3{0.0f, 0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{0.0f, 0.0f, 1.0f, 0.0f});
   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].position ==
           float3{2.0f, 2.0f, 2.0f});
   REQUIRE(edit_context.euler_rotation == float3{4.0f, 4.0f, 4.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_region_metrics coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::region{};

   set_creation_region_metrics edit{quaternion{0.0f, 1.0f, 0.0f, 0.0f},
                                    quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                    float3{1.0f, 1.0f, 1.0f},
                                    float3{0.0f, 0.0f, 0.0f},
                                    float3{2.0f, 2.0f, 2.0f},
                                    float3{0.0f, 0.0f, 0.0f}};
   set_creation_region_metrics other_edit{quaternion{0.0f, 0.0f, 1.0f, 0.0f},
                                          quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                          float3{2.0f, 2.0f, 2.0f},
                                          float3{0.0f, 0.0f, 0.0f},
                                          float3{4.0f, 4.0f, 4.0f},
                                          float3{0.0f, 0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).rotation ==
           quaternion{0.0f, 0.0f, 1.0f, 0.0f});
   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).position ==
           float3{2.0f, 2.0f, 2.0f});
   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).size ==
           float3{4.0f, 4.0f, 4.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).position ==
           float3{0.0f, 0.0f, 0.0f});
   REQUIRE(std::get<world::region>(*interaction_targets.creation_entity).size ==
           float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits set_creation_sector_point coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::sector{.points = {{0.0f, 0.0f}}};

   set_creation_sector_point edit{float2{1.0f, 1.0f}, float2{0.0f, 0.0f}};
   set_creation_sector_point other_edit{float2{2.0f, 2.0f}, float2{0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::sector>(*interaction_targets.creation_entity).points[0] ==
           float2{2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::sector>(*interaction_targets.creation_entity).points[0] ==
           float2{0.0f, 0.0f});
}

TEST_CASE("edits set_creation_portal_size coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::portal{};

   set_creation_portal_size edit{2.0f, 1.0f, 4.0f, 2.0f};
   set_creation_portal_size other_edit{8.0f, 2.0f, 16.0f, 4.0f};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::portal>(*interaction_targets.creation_entity).width == 8.0f);
   REQUIRE(std::get<world::portal>(*interaction_targets.creation_entity).height == 16.0f);

   edit.revert(edit_context);

   REQUIRE(std::get<world::portal>(*interaction_targets.creation_entity).width == 1.0f);
   REQUIRE(std::get<world::portal>(*interaction_targets.creation_entity).height == 2.0f);
}

TEST_CASE("edits set_creation_barrier_metrics coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::barrier{};

   set_creation_barrier_metrics edit{1.0f,
                                     0.0f,
                                     float2{1.0f, 1.0f},
                                     float2{0.0f, 0.0f},
                                     float2{2.0f, 2.0f},
                                     float2{0.0f, 0.0f}};
   set_creation_barrier_metrics other_edit{2.0f,
                                           1.0f,
                                           float2{2.0f, 2.0f},
                                           float2{0.0f, 0.0f},
                                           float2{4.0f, 4.0f},
                                           float2{0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).rotation_angle ==
           2.0f);
   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).position ==
           float2{2.0f, 2.0f});
   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).size ==
           float2{4.0f, 4.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).rotation_angle ==
           0.0f);
   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).position ==
           float2{0.0f, 0.0f});
   REQUIRE(std::get<world::barrier>(*interaction_targets.creation_entity).size ==
           float2{0.0f, 0.0f});
}

}
