#include "pch.h"

#include "edits/ui_action.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

#include <memory>

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits ui_edit", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit edit{world.objects[0].id, &world::object::team, 1, world.objects[0].team};

   edit.apply(edit_context);

   REQUIRE(world.objects[0].team == 1);

   edit.revert(edit_context);

   REQUIRE(world.objects[0].team == 0);
}

TEST_CASE("edits ui_edit_indexed", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_indexed edit{world.objects[0].id, &world::object::instance_properties, 0,
                        world::instance_property{.key = "MaxHealth"s, .value = "10"s},
                        world.objects[0].instance_properties[0]};

   edit.apply(edit_context);

   REQUIRE(world.objects[0].instance_properties[0].value == "10");

   edit.revert(edit_context);

   REQUIRE(world.objects[0].instance_properties[0].value == "50000");
}

TEST_CASE("edits ui_edit_path_node", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_path_node edit{world.paths[0].id, 0, &world::path::node::position,
                          float3{-1.0f, -1.0f, -1.0f},
                          world.paths[0].nodes[0].position};

   edit.apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{-1.0f, -1.0f, -1.0f});

   edit.revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits ui_edit_path_node_indexed", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_path_node_indexed edit{world.paths[0].id,
                                  0,
                                  &world::path::node::properties,
                                  0,
                                  world::path::property{.key = "Key"s, .value = "NewValue"s},
                                  world.paths[0].nodes[0].properties[0]};

   edit.apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].properties[0].value == "NewValue");

   edit.revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].properties[0].value == "Value");
}

TEST_CASE("edits ui_creation_edit", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_edit edit{&world::object::layer, 1, 0};

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 1);

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
}

TEST_CASE("edits ui_creation_edit_with_meta", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_edit_with_meta edit{&world::object::layer,
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

TEST_CASE("edits ui_creation_path_node_edit", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   ui_creation_path_node_edit edit{&world::path::node::rotation,
                                   {-1.0f, 0.0f, 0.0f, 0.0f},
                                   {1.0f, 0.0f, 0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{-1.0f, 0.0f, 0.0f, 0.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits ui_creation_path_node_edit_with_meta", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   ui_creation_path_node_edit_with_meta edit{&world::path::node::rotation,
                                             {-1.0f, 0.0f, 0.0f, 0.0f},
                                             {1.0f, 0.0f, 0.0f, 0.0f},
                                             &world::edit_context::euler_rotation,
                                             {1.0f, 1.0f, 1.0f},
                                             {0.0f, 0.0f, 0.0f}};

   edit.apply(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{-1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{1.0f, 1.0f, 1.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits ui_edit coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit edit{world.objects[0].id, &world::object::team, 1, world.objects[0].team};
   ui_edit other_edit{world.objects[0].id, &world::object::team, 2,
                      world.objects[0].team};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(world.objects[0].team == 2);

   edit.revert(edit_context);

   REQUIRE(world.objects[0].team == 0);
}

TEST_CASE("edits ui_edit_indexed coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_indexed edit{world.objects[0].id, &world::object::instance_properties, 0,
                        world::instance_property{.key = "MaxHealth"s, .value = "10"s},
                        world.objects[0].instance_properties[0]};
   ui_edit_indexed other_edit{world.objects[0].id,
                              &world::object::instance_properties, 0,
                              world::instance_property{.key = "MaxHealth"s,
                                                       .value = "40"s},
                              world.objects[0].instance_properties[0]};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(world.objects[0].instance_properties[0].value == "40");

   edit.revert(edit_context);

   REQUIRE(world.objects[0].instance_properties[0].value == "50000");
}

TEST_CASE("edits ui_edit_path_node coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_path_node edit{world.paths[0].id, 0, &world::path::node::position,
                          float3{-1.0f, -1.0f, -1.0f},
                          world.paths[0].nodes[0].position};
   ui_edit_path_node other_edit{world.paths[0].id, 0, &world::path::node::position,
                                float3{-4.0f, -4.0f, -4.0f},
                                world.paths[0].nodes[0].position};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{-4.0f, -4.0f, -4.0f});

   edit.revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].position == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits ui_edit_path_node_indexed coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_path_node_indexed edit{world.paths[0].id,
                                  0,
                                  &world::path::node::properties,
                                  0,
                                  world::path::property{.key = "Key"s, .value = "NewValue"s},
                                  world.paths[0].nodes[0].properties[0]};
   ui_edit_path_node_indexed other_edit{world.paths[0].id,
                                        0,
                                        &world::path::node::properties,
                                        0,
                                        world::path::property{.key = "Key"s,
                                                              .value = "CoalescedValue"s},
                                        world.paths[0].nodes[0].properties[0]};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(world.paths[0].nodes[0].properties[0].value == "CoalescedValue");

   edit.revert(edit_context);

   REQUIRE(world.paths[0].nodes[0].properties[0].value == "Value");
}

TEST_CASE("edits ui_creation_edit coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_edit edit{&world::object::layer, 1, 0};
   ui_creation_edit other_edit{&world::object::layer, 4, 0};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 4);

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
}

TEST_CASE("edits ui_creation_edit_with_meta coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_edit_with_meta edit{&world::object::layer,
                                   1,
                                   0,
                                   &world::edit_context::euler_rotation,
                                   {1.0f, 1.0f, 1.0f},
                                   {0.0f, 0.0f, 0.0f}};
   ui_creation_edit_with_meta other_edit{&world::object::layer,
                                         4,
                                         0,
                                         &world::edit_context::euler_rotation,
                                         {2.0f, 2.0f, 2.0f},
                                         {0.0f, 0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 4);
   REQUIRE(edit_context.euler_rotation == float3{2.0f, 2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::object>(*interaction_targets.creation_entity).layer == 0);
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits ui_creation_path_node_edit coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   ui_creation_path_node_edit edit{&world::path::node::rotation,
                                   {-1.0f, 0.0f, 0.0f, 0.0f},
                                   {1.0f, 0.0f, 0.0f, 0.0f}};
   ui_creation_path_node_edit other_edit{&world::path::node::rotation,
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

TEST_CASE("edits ui_creation_path_node_edit_with_meta coalesce", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::path{.nodes = {world::path::node{}}};

   ui_creation_path_node_edit_with_meta edit{&world::path::node::rotation,
                                             {-1.0f, 0.0f, 0.0f, 0.0f},
                                             {1.0f, 0.0f, 0.0f, 0.0f},
                                             &world::edit_context::euler_rotation,
                                             {1.0f, 1.0f, 1.0f},
                                             {0.0f, 0.0f, 0.0f}};
   ui_creation_path_node_edit_with_meta other_edit{&world::path::node::rotation,
                                                   {0.0f, 1.0f, 0.0f, 0.0f},
                                                   {1.0f, 0.0f, 0.0f, 0.0f},
                                                   &world::edit_context::euler_rotation,
                                                   {2.0f, 2.0f, 2.0f},
                                                   {0.0f, 0.0f, 0.0f}};

   REQUIRE(edit.is_coalescable(other_edit));

   edit.coalesce(other_edit);

   edit.apply(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{2.0f, 2.0f, 2.0f});

   edit.revert(edit_context);

   REQUIRE(std::get<world::path>(*interaction_targets.creation_entity).nodes[0].rotation ==
           quaternion{1.0f, 0.0f, 0.0f, 0.0f});
   REQUIRE(edit_context.euler_rotation == float3{0.0f, 0.0f, 0.0f});
}

TEST_CASE("edits ui_edit not coalescable", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit edit{world.objects[0].id, &world::object::team, 1, world.objects[0].team};
   ui_edit other_edit{world.objects[0].id, &world::object::layer, 2,
                      world.objects[0].team};

   REQUIRE(not edit.is_coalescable(other_edit));
}

TEST_CASE("edits ui_edit_indexed not coalescable", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_indexed edit{world.objects[0].id, &world::object::instance_properties, 0,
                        world::instance_property{.key = "MaxHealth"s, .value = "10"s},
                        world.objects[0].instance_properties[0]};
   ui_edit_indexed other_edit{world.objects[0].id,
                              &world::object::instance_properties, 1,
                              world::instance_property{.key = "MaxHealth"s,
                                                       .value = "40"s},
                              world.objects[0].instance_properties[0]};

   REQUIRE(not edit.is_coalescable(other_edit));
}

TEST_CASE("edits ui_edit_path_node not coalescable", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_path_node edit{world.paths[0].id, 0, &world::path::node::position,
                          float3{-1.0f, -1.0f, -1.0f},
                          world.paths[0].nodes[0].position};
   ui_edit_path_node other_edit{world.paths[0].id, 1, &world::path::node::position,
                                float3{-4.0f, -4.0f, -4.0f},
                                world.paths[0].nodes[0].position};

   REQUIRE(not edit.is_coalescable(other_edit));
}

TEST_CASE("edits ui_edit_path_node_indexed not coalescable", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   ui_edit_path_node_indexed edit{world.paths[0].id,
                                  0,
                                  &world::path::node::properties,
                                  0,
                                  world::path::property{.key = "Key"s, .value = "NewValue"s},
                                  world.paths[0].nodes[0].properties[0]};
   ui_edit_path_node_indexed other_edit{world.paths[0].id,
                                        1,
                                        &world::path::node::properties,
                                        0,
                                        world::path::property{.key = "Key"s,
                                                              .value = "NewValue"s},
                                        world.paths[0].nodes[0].properties[0]};

   REQUIRE(not edit.is_coalescable(other_edit));
}

TEST_CASE("edits ui_creation_edit not coalescable", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_edit edit{&world::object::layer, 1, 0};
   ui_creation_edit other_edit{&world::object::team, 4, 0};

   REQUIRE(not edit.is_coalescable(other_edit));
}

TEST_CASE("edits ui_creation_edit_with_meta not coalescable", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_edit_with_meta edit{&world::object::layer,
                                   1,
                                   0,
                                   &world::edit_context::euler_rotation,
                                   {1.0f, 1.0f, 1.0f},
                                   {0.0f, 0.0f, 0.0f}};
   ui_creation_edit_with_meta other_edit{&world::object::team,
                                         4,
                                         0,
                                         &world::edit_context::euler_rotation,
                                         {2.0f, 2.0f, 2.0f},
                                         {0.0f, 0.0f, 0.0f}};

   REQUIRE(not edit.is_coalescable(other_edit));
}

TEST_CASE("edits ui_creation_path_node_edit not coalescable", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_path_node_edit edit{&world::path::node::position, {}, {}};
   ui_creation_path_node_edit other_edit{&world::path::node::rotation, {}, {}};

   REQUIRE(not edit.is_coalescable(other_edit));
}

TEST_CASE("edits ui_creation_path_node_edit_with_meta not coalescable",
          "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   interaction_targets.creation_entity = world::object{};

   ui_creation_path_node_edit_with_meta edit{&world::path::node::position,
                                             {1.0f, 1.0f, 1.0f},
                                             {0.0f, 0.0f, 0.0f},
                                             &world::edit_context::euler_rotation,
                                             {1.0f, 1.0f, 1.0f},
                                             {0.0f, 0.0f, 0.0f}};
   ui_creation_path_node_edit_with_meta other_edit{&world::path::node::rotation,
                                                   {-1.0f, 0.0f, 0.0f, 0.0f},
                                                   {1.0f, 0.0f, 0.0f, 0.0f},
                                                   &world::edit_context::euler_rotation,
                                                   {2.0f, 2.0f, 2.0f},
                                                   {0.0f, 0.0f, 0.0f}};

   REQUIRE(not edit.is_coalescable(other_edit));
}

}
