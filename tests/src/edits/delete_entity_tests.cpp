#include "pch.h"

#include "edits/delete_entity.hpp"
#include "world/object_class_library.hpp"
#include "world/world.hpp"

#include "null_asset_libraries.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits delete_entity object", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_entity(world.objects[0].id, world, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.objects.empty());

   REQUIRE(world.paths[0].properties.size() == 1);

   REQUIRE(world.sectors[0].objects.empty());

   REQUIRE(world.hintnodes[0].command_post == "");

   edit->revert(edit_context);

   REQUIRE(world.objects.size() == 1);
   CHECK(world.objects[0].name == test_world.objects[0].name);
   CHECK(world.objects[0].layer == test_world.objects[0].layer);
   CHECK(world.objects[0].hidden == test_world.objects[0].hidden);
   CHECK(world.objects[0].rotation == test_world.objects[0].rotation);
   CHECK(world.objects[0].position == test_world.objects[0].position);
   CHECK(world.objects[0].team == test_world.objects[0].team);
   CHECK(world.objects[0].class_name == test_world.objects[0].class_name);
   CHECK(world.objects[0].instance_properties ==
         test_world.objects[0].instance_properties);

   REQUIRE(world.paths[0].properties.size() == 2);
   REQUIRE(world.paths[0].properties[1].key == "EnableObject");
   REQUIRE(world.paths[0].properties[1].value == "test_object");

   REQUIRE(world.sectors[0].objects.size() == 1);
   REQUIRE(world.sectors[0].objects[0] == "test_object");

   REQUIRE(world.hintnodes[0].command_post == "test_object");
}

TEST_CASE("edits delete_entity light", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.lights[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.lights.empty());

   edit->revert(edit_context);

   REQUIRE(world.lights.size() == 1);
   REQUIRE(world.lights[0] == test_world.lights[0]);
}

TEST_CASE("edits delete_entity path", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto delete_node_0 = make_delete_entity(world.paths[0].id, 0, world);

   delete_node_0->apply(edit_context);

   REQUIRE(not world.paths.empty());
   REQUIRE(world.paths[0].nodes.size() == 1);
   REQUIRE(world.paths[0].nodes[0] == test_world.paths[0].nodes[1]);

   REQUIRE(world.objects[0].instance_properties.size() == 9);

   auto delete_node_1 = make_delete_entity(world.paths[0].id, 1, world);

   delete_node_1->apply(edit_context);

   REQUIRE(world.paths.empty());

   REQUIRE(world.objects[0].instance_properties.size() == 9);
   REQUIRE(world.objects[0].instance_properties[1].key == "SpawnPath");
   REQUIRE(world.objects[0].instance_properties[1].value == "");
   REQUIRE(world.objects[0].instance_properties[2].key == "AllyPath");
   REQUIRE(world.objects[0].instance_properties[2].value == "");
   REQUIRE(world.objects[0].instance_properties[3].key == "TurretPath");
   REQUIRE(world.objects[0].instance_properties[3].value == "");

   delete_node_1->revert(edit_context);

   REQUIRE(not world.paths.empty());
   REQUIRE(world.paths[0].nodes.size() == 1);
   REQUIRE(world.paths[0].nodes[0] == test_world.paths[0].nodes[1]);

   REQUIRE(world.objects[0].instance_properties.size() == 9);
   REQUIRE(world.objects[0].instance_properties[1].key == "SpawnPath");
   REQUIRE(world.objects[0].instance_properties[1].value == "test_path");
   REQUIRE(world.objects[0].instance_properties[2].key == "AllyPath");
   REQUIRE(world.objects[0].instance_properties[2].value == "test_path");
   REQUIRE(world.objects[0].instance_properties[3].key == "TurretPath");
   REQUIRE(world.objects[0].instance_properties[3].value == "test_path");

   delete_node_0->revert(edit_context);

   REQUIRE(not world.paths.empty());
   REQUIRE(world.paths[0] == test_world.paths[0]);
}

TEST_CASE("edits delete_entity region", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.regions[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.regions.empty());

   REQUIRE(world.objects[0].instance_properties.size() == 9);
   REQUIRE(world.objects[0].instance_properties[4].key == "CaptureRegion");
   REQUIRE(world.objects[0].instance_properties[4].value == "");
   REQUIRE(world.objects[0].instance_properties[5].key == "ControlRegion");
   REQUIRE(world.objects[0].instance_properties[5].value == "");
   REQUIRE(world.objects[0].instance_properties[6].key == "EffectRegion");
   REQUIRE(world.objects[0].instance_properties[6].value == "");
   REQUIRE(world.objects[0].instance_properties[7].key == "KillRegion");
   REQUIRE(world.objects[0].instance_properties[7].value == "");
   REQUIRE(world.objects[0].instance_properties[8].key == "SpawnRegion");
   REQUIRE(world.objects[0].instance_properties[8].value == "");

   edit->revert(edit_context);

   REQUIRE(world.regions.size() == 1);
   REQUIRE(world.regions[0] == test_world.regions[0]);

   REQUIRE(world.objects[0].instance_properties.size() == 9);
   REQUIRE(world.objects[0].instance_properties[4].key == "CaptureRegion");
   REQUIRE(world.objects[0].instance_properties[4].value == "test_region");
   REQUIRE(world.objects[0].instance_properties[5].key == "ControlRegion");
   REQUIRE(world.objects[0].instance_properties[5].value == "test_region");
   REQUIRE(world.objects[0].instance_properties[6].key == "EffectRegion");
   REQUIRE(world.objects[0].instance_properties[6].value == "test_region");
   REQUIRE(world.objects[0].instance_properties[7].key == "KillRegion");
   REQUIRE(world.objects[0].instance_properties[7].value == "test_region");
   REQUIRE(world.objects[0].instance_properties[8].key == "SpawnRegion");
   REQUIRE(world.objects[0].instance_properties[8].value == "test_region");
}

TEST_CASE("edits delete_entity sector", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.sectors[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.sectors.empty());

   REQUIRE(world.portals[0].sector1 == "");

   edit->revert(edit_context);

   REQUIRE(world.sectors.size() == 1);
   REQUIRE(world.sectors[0] == test_world.sectors[0]);

   REQUIRE(world.portals[0].sector1 == "test_sector");
}

TEST_CASE("edits delete_entity portal", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.portals[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.portals.empty());

   edit->revert(edit_context);

   REQUIRE(world.portals.size() == 1);
   REQUIRE(world.portals[0] == test_world.portals[0]);
}

TEST_CASE("edits delete_entity hintnode", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.hintnodes[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.hintnodes.empty());

   edit->revert(edit_context);

   REQUIRE(world.hintnodes.size() == 1);
   REQUIRE(world.hintnodes[0] == test_world.hintnodes[0]);
}

TEST_CASE("edits delete_entity barrier", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.barriers[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.barriers.empty());

   edit->revert(edit_context);

   REQUIRE(world.barriers.size() == 1);
   REQUIRE(world.barriers[0] == test_world.barriers[0]);
}

TEST_CASE("edits delete_entity planning_hub", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.planning_hubs[1].id, world);

   edit->apply(edit_context);

   REQUIRE(world.planning_hubs.size() == 2);
   CHECK(world.planning_hubs[0] == test_world.planning_hubs[0]);
   CHECK(world.planning_hubs[1] == test_world.planning_hubs[2]);

   REQUIRE(world.planning_connections.size() == 1);
   CHECK(world.planning_connections[0].name == "Connection1");
   CHECK(world.planning_connections[0].start_hub_index == 0);
   CHECK(world.planning_connections[0].end_hub_index == 1);

   edit->revert(edit_context);

   CHECK(world.planning_hubs == test_world.planning_hubs);

   REQUIRE(world.planning_connections.size() == 3);
   CHECK(world.planning_connections[0].name == "Connection0");
   CHECK(world.planning_connections[0].start_hub_index == 0);
   CHECK(world.planning_connections[0].end_hub_index == 1);
   CHECK(world.planning_connections[1].name == "Connection1");
   CHECK(world.planning_connections[1].start_hub_index == 0);
   CHECK(world.planning_connections[1].end_hub_index == 2);
   CHECK(world.planning_connections[2].name == "Connection2");
   CHECK(world.planning_connections[2].start_hub_index == 1);
   CHECK(world.planning_connections[2].end_hub_index == 2);
}

TEST_CASE("edits delete_entity planning_hub first", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.planning_hubs[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.planning_hubs.size() == 2);
   CHECK(world.planning_hubs[0] == test_world.planning_hubs[1]);
   CHECK(world.planning_hubs[1] == test_world.planning_hubs[2]);

   REQUIRE(world.planning_connections.size() == 1);
   CHECK(world.planning_connections[0].name == "Connection2");
   CHECK(world.planning_connections[0].start_hub_index == 0);
   CHECK(world.planning_connections[0].end_hub_index == 1);

   edit->revert(edit_context);

   CHECK(world.planning_hubs == test_world.planning_hubs);

   REQUIRE(world.planning_connections.size() == 3);
   CHECK(world.planning_connections[0].name == "Connection0");
   CHECK(world.planning_connections[0].start_hub_index == 0);
   CHECK(world.planning_connections[0].end_hub_index == 1);
   CHECK(world.planning_connections[1].name == "Connection1");
   CHECK(world.planning_connections[1].start_hub_index == 0);
   CHECK(world.planning_connections[1].end_hub_index == 2);
   CHECK(world.planning_connections[2].name == "Connection2");
   CHECK(world.planning_connections[2].start_hub_index == 1);
   CHECK(world.planning_connections[2].end_hub_index == 2);
}

TEST_CASE("edits delete_entity planning_hub last", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.planning_hubs[2].id, world);

   edit->apply(edit_context);

   REQUIRE(world.planning_hubs.size() == 2);
   CHECK(world.planning_hubs[0] == test_world.planning_hubs[0]);
   CHECK(world.planning_hubs[1] == test_world.planning_hubs[1]);

   REQUIRE(world.planning_connections.size() == 1);
   CHECK(world.planning_connections[0].name == "Connection0");
   CHECK(world.planning_connections[0].start_hub_index == 0);
   CHECK(world.planning_connections[0].end_hub_index == 1);

   edit->revert(edit_context);

   CHECK(world.planning_hubs == test_world.planning_hubs);

   REQUIRE(world.planning_connections.size() == 3);
   CHECK(world.planning_connections[0].name == "Connection0");
   CHECK(world.planning_connections[0].start_hub_index == 0);
   CHECK(world.planning_connections[0].end_hub_index == 1);
   CHECK(world.planning_connections[1].name == "Connection1");
   CHECK(world.planning_connections[1].start_hub_index == 0);
   CHECK(world.planning_connections[1].end_hub_index == 2);
   CHECK(world.planning_connections[2].name == "Connection2");
   CHECK(world.planning_connections[2].start_hub_index == 1);
   CHECK(world.planning_connections[2].end_hub_index == 2);
}

TEST_CASE("edits delete_entity planning_hub weights", "[Edits]")
{
   using world::planning_branch_weights;

   world::world
      world =
         {
            .planning_hubs =
               {
                  world::entities_init,
                  std::initializer_list{
                     world::planning_hub{.name = "Hub0",
                                         .position = float3{-63.822487f, 0.0f, -9.202278f},
                                         .radius = 8.0f,
                                         .id = world::planning_hub_id{0}},

                     world::planning_hub{.name = "Hub1",
                                         .position = float3{-121.883095f, 0.0f, -30.046543f},
                                         .radius = 7.586431f,
                                         .weights =
                                            {
                                               planning_branch_weights{
                                                  .hub_index = 3,
                                                  .connection_index = 2,
                                                  .soldier = 25.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 0,
                                                  .connection_index = 2,
                                                  .soldier = 75.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 0,
                                                  .connection_index = 3,
                                                  .soldier = 75.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 1,
                                                  .connection_index = 4,
                                                  .soldier = 50.0f,
                                               },
                                            },
                                         .id = world::planning_hub_id{1}},

                     world::planning_hub{
                        .name = "Hub2",
                        .position = float3{-121.883095f, 0.0f, -60.046543f},
                        .radius = 7.586431f,
                        .id = world::planning_hub_id{2},
                     },

                     world::planning_hub{.name = "Hub3",
                                         .position = float3{-121.883095f, 0.0f, -60.046543f},
                                         .radius = 7.586431f,
                                         .weights =
                                            {
                                               planning_branch_weights{
                                                  .hub_index = 2,
                                                  .connection_index = 4,
                                                  .soldier = 25.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 1,
                                                  .connection_index = 3,
                                                  .soldier = 75.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 1,
                                                  .connection_index = 3,
                                                  .soldier = 50.0f,
                                               },
                                            },
                                         .id = world::planning_hub_id{3}},
                  },
               },

            .planning_connections =
               {
                  world::entities_init,
                  std::initializer_list{
                     world::planning_connection{.name = "Connection0",
                                                .start_hub_index = 0,
                                                .end_hub_index = 1,
                                                .id = world::planning_connection_id{0}},
                     world::planning_connection{.name = "Connection1",
                                                .start_hub_index = 0,
                                                .end_hub_index = 2,
                                                .id = world::planning_connection_id{1}},
                     world::planning_connection{.name = "Connection2",
                                                .start_hub_index = 1,
                                                .end_hub_index = 2,
                                                .id = world::planning_connection_id{2}},
                     world::planning_connection{.name = "Connection3",
                                                .start_hub_index = 2,
                                                .end_hub_index = 3,
                                                .id = world::planning_connection_id{3}},
                     world::planning_connection{.name = "Connection4",
                                                .start_hub_index = 3,
                                                .end_hub_index = 0,
                                                .id = world::planning_connection_id{4}},
                  },
               },
         };

   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.planning_hubs[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.planning_hubs.size() == 3);
   REQUIRE(world.planning_hubs[0].weights.size() == 1);
   CHECK(world.planning_hubs[0].weights[0] ==
         planning_branch_weights{.hub_index = 2, .connection_index = 0, .soldier = 25.0f});

   REQUIRE(world.planning_hubs[2].weights.size() == 2);
   CHECK(world.planning_hubs[2].weights[0] ==
         planning_branch_weights{.hub_index = 0, .connection_index = 1, .soldier = 75.0f});
   CHECK(world.planning_hubs[2].weights[1] ==
         planning_branch_weights{.hub_index = 0, .connection_index = 1, .soldier = 50.0f});

   CHECK(world.planning_connections.size() == 2);

   edit->revert(edit_context);

   REQUIRE(world.planning_hubs.size() == 4);

   REQUIRE(world.planning_hubs[1].weights.size() == 4);
   CHECK(world.planning_hubs[1].weights[0] ==
         planning_branch_weights{.hub_index = 3, .connection_index = 2, .soldier = 25.0f});
   CHECK(world.planning_hubs[1].weights[1] ==
         planning_branch_weights{.hub_index = 0, .connection_index = 2, .soldier = 75.0f});
   CHECK(world.planning_hubs[1].weights[2] ==
         planning_branch_weights{.hub_index = 0, .connection_index = 3, .soldier = 75.0f});
   CHECK(world.planning_hubs[1].weights[3] ==
         planning_branch_weights{.hub_index = 1, .connection_index = 4, .soldier = 50.0f});

   REQUIRE(world.planning_hubs[3].weights.size() == 3);
   CHECK(world.planning_hubs[3].weights[0] ==
         planning_branch_weights{.hub_index = 2, .connection_index = 4, .soldier = 25.0f});
   CHECK(world.planning_hubs[3].weights[1] ==
         planning_branch_weights{.hub_index = 1, .connection_index = 3, .soldier = 75.0f});
   CHECK(world.planning_hubs[3].weights[2] ==
         planning_branch_weights{.hub_index = 1, .connection_index = 3, .soldier = 50.0f});
}

TEST_CASE("edits delete_entity planning_connection", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.planning_connections[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.planning_connections.size() == 2);
   CHECK(world.planning_connections[0] == test_world.planning_connections[1]);
   CHECK(world.planning_connections[1] == test_world.planning_connections[2]);

   edit->revert(edit_context);

   REQUIRE(world.planning_connections.size() == 3);
   CHECK(world.planning_connections == test_world.planning_connections);
}

TEST_CASE("edits delete_entity planning_connection weights", "[Edits]")
{
   using world::planning_branch_weights;

   world::world
      world =
         {
            .planning_hubs =
               {
                  world::entities_init,
                  std::initializer_list{
                     world::planning_hub{.name = "Hub0",
                                         .position = float3{-63.822487f, 0.0f, -9.202278f},
                                         .radius = 8.0f,
                                         .id = world::planning_hub_id{0}},

                     world::planning_hub{.name = "Hub1",
                                         .position = float3{-121.883095f, 0.0f, -30.046543f},
                                         .radius = 7.586431f,
                                         .weights =
                                            {
                                               planning_branch_weights{
                                                  .hub_index = 3,
                                                  .connection_index = 2,
                                                  .soldier = 25.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 0,
                                                  .connection_index = 2,
                                                  .soldier = 75.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 0,
                                                  .connection_index = 3,
                                                  .soldier = 75.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 1,
                                                  .connection_index = 4,
                                                  .soldier = 50.0f,
                                               },
                                            },
                                         .id = world::planning_hub_id{1}},

                     world::planning_hub{
                        .name = "Hub2",
                        .position = float3{-121.883095f, 0.0f, -60.046543f},
                        .radius = 7.586431f,
                        .id = world::planning_hub_id{2},
                     },

                     world::planning_hub{.name = "Hub3",
                                         .position = float3{-121.883095f, 0.0f, -60.046543f},
                                         .radius = 7.586431f,
                                         .weights =
                                            {
                                               planning_branch_weights{
                                                  .hub_index = 2,
                                                  .connection_index = 4,
                                                  .soldier = 25.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 1,
                                                  .connection_index = 3,
                                                  .soldier = 75.0f,
                                               },
                                               planning_branch_weights{
                                                  .hub_index = 1,
                                                  .connection_index = 3,
                                                  .soldier = 50.0f,
                                               },
                                            },
                                         .id = world::planning_hub_id{3}},
                  },
               },

            .planning_connections =
               {
                  world::entities_init,
                  std::initializer_list{
                     world::planning_connection{.name = "Connection0",
                                                .start_hub_index = 0,
                                                .end_hub_index = 1,
                                                .id = world::planning_connection_id{0}},
                     world::planning_connection{.name = "Connection1",
                                                .start_hub_index = 0,
                                                .end_hub_index = 2,
                                                .id = world::planning_connection_id{1}},
                     world::planning_connection{.name = "Connection2",
                                                .start_hub_index = 1,
                                                .end_hub_index = 2,
                                                .id = world::planning_connection_id{2}},
                     world::planning_connection{.name = "Connection3",
                                                .start_hub_index = 2,
                                                .end_hub_index = 3,
                                                .id = world::planning_connection_id{3}},
                     world::planning_connection{.name = "Connection4",
                                                .start_hub_index = 3,
                                                .end_hub_index = 0,
                                                .id = world::planning_connection_id{4}},
                  },
               },
         };

   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.planning_connections[3].id, world);

   edit->apply(edit_context);

   REQUIRE(world.planning_hubs[1].weights.size() == 3);
   CHECK(world.planning_hubs[1].weights[0] ==
         planning_branch_weights{.hub_index = 3, .connection_index = 2, .soldier = 25.0f});
   CHECK(world.planning_hubs[1].weights[1] ==
         planning_branch_weights{.hub_index = 0, .connection_index = 2, .soldier = 75.0f});
   CHECK(world.planning_hubs[1].weights[2] ==
         planning_branch_weights{.hub_index = 1, .connection_index = 3, .soldier = 50.0f});

   REQUIRE(world.planning_hubs[3].weights.size() == 1);
   CHECK(world.planning_hubs[3].weights[0] ==
         planning_branch_weights{.hub_index = 2, .connection_index = 3, .soldier = 25.0f});

   edit->revert(edit_context);

   REQUIRE(world.planning_hubs[1].weights.size() == 4);
   CHECK(world.planning_hubs[1].weights[0] ==
         planning_branch_weights{.hub_index = 3, .connection_index = 2, .soldier = 25.0f});
   CHECK(world.planning_hubs[1].weights[1] ==
         planning_branch_weights{.hub_index = 0, .connection_index = 2, .soldier = 75.0f});
   CHECK(world.planning_hubs[1].weights[2] ==
         planning_branch_weights{.hub_index = 0, .connection_index = 3, .soldier = 75.0f});
   CHECK(world.planning_hubs[1].weights[3] ==
         planning_branch_weights{.hub_index = 1, .connection_index = 4, .soldier = 50.0f});

   REQUIRE(world.planning_hubs[3].weights.size() == 3);
   CHECK(world.planning_hubs[3].weights[0] ==
         planning_branch_weights{.hub_index = 2, .connection_index = 4, .soldier = 25.0f});
   CHECK(world.planning_hubs[3].weights[1] ==
         planning_branch_weights{.hub_index = 1, .connection_index = 3, .soldier = 75.0f});
   CHECK(world.planning_hubs[3].weights[2] ==
         planning_branch_weights{.hub_index = 1, .connection_index = 3, .soldier = 50.0f});
}

TEST_CASE("edits delete_entity boundary", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.boundaries[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.boundaries.empty());

   edit->revert(edit_context);

   REQUIRE(world.boundaries.size() == 1);
   REQUIRE(world.boundaries[0] == test_world.boundaries[0]);
}

TEST_CASE("edits delete_entity measurement", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.measurements[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.measurements.empty());

   edit->revert(edit_context);

   REQUIRE(world.measurements.size() == 1);
   REQUIRE(world.measurements[0] == test_world.measurements[0]);
}

TEST_CASE("edits delete_entity multiple refs object", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .requirements = {{.file_type = "world", .entries = {"Test"}}},

      .layer_descriptions = {{.name = "[Base]"s}},
      .common_layers = {0},

      .terrain = {},
      .global_lights = {.env_map_texture = "sky"},

      .objects =
         {
            world::entities_init,
            std::initializer_list{
               world::object{
                  .name = "test_object"s,

                  .id = world::object_id{0},
               },
               world::object{
                  .name = "other_object"s,

                  .id = world::object_id{1},
               },
            },
         },

      .paths =
         {
            world::entities_init,
            std::initializer_list{
               world::path{
                  .name = "test_path"s,

                  .properties =
                     {
                        {.key = "Key"s, .value = "Value"s},
                        {.key = "EnableObject"s, .value = "test_object"s},
                        {.key = "Key"s, .value = "Value"s},
                        {.key = "EnableObject"s, .value = "test_object"s},
                        {.key = "EnableObject"s, .value = "test_object"s},
                     },
               },
            },
         },

      .sectors =
         {
            world::entities_init,
            std::initializer_list{
               world::sector{
                  .name = "test_sector"s,

                  .base = 0.0f,
                  .height = 0.0f,
                  .points = {{0.0f, 0.0f}, {0.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 0.0f}},
                  .objects = {"other_object"s, "test_object"s, "other_object"s, "test_object"s},
               },
            },
         },
   };

   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_entity(world.objects[0].id, world, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.paths[0].properties.size() == 2);
   CHECK(world.paths[0].properties[0] ==
         world::path::property{.key = "Key"s, .value = "Value"s});
   CHECK(world.paths[0].properties[1] ==
         world::path::property{.key = "Key"s, .value = "Value"s});

   REQUIRE(world.sectors[0].objects.size() == 2);
   CHECK(world.sectors[0].objects[0] == "other_object");
   CHECK(world.sectors[0].objects[1] == "other_object");

   edit->revert(edit_context);

   REQUIRE(world.paths[0].properties.size() == 5);
   CHECK(world.paths[0].properties[0] ==
         world::path::property{.key = "Key"s, .value = "Value"s});
   CHECK(world.paths[0].properties[1] ==
         world::path::property{.key = "EnableObject"s, .value = "test_object"s});
   CHECK(world.paths[0].properties[2] ==
         world::path::property{.key = "Key"s, .value = "Value"s});
   CHECK(world.paths[0].properties[3] ==
         world::path::property{.key = "EnableObject"s, .value = "test_object"s});
   CHECK(world.paths[0].properties[4] ==
         world::path::property{.key = "EnableObject"s, .value = "test_object"s});

   REQUIRE(world.sectors[0].objects.size() == 4);
   CHECK(world.sectors[0].objects[0] == "other_object");
   CHECK(world.sectors[0].objects[1] == "test_object");
   CHECK(world.sectors[0].objects[2] == "other_object");
   CHECK(world.sectors[0].objects[3] == "test_object");
}

TEST_CASE("edits delete_entity ControlZone ref object", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .layer_descriptions = {{.name = "[Base]"s}},
      .common_layers = {0},

      .objects =
         {
            world::entities_init,
            std::initializer_list{
               world::object{
                  .name = "test_object"s,

                  .id = world::object_id{0},
               },
               world::object{
                  .instance_properties =
                     {world::instance_property{"ControlZone", "test_object"}},

                  .id = world::object_id{1},
               },
            },
         },
   };

   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_entity(world.objects[0].id, world, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.objects.size() == 1);
   REQUIRE(world.objects[0].instance_properties.size() == 1);
   CHECK(world.objects[0].instance_properties[0].key == "ControlZone");
   CHECK(world.objects[0].instance_properties[0].value.empty());

   edit->revert(edit_context);

   REQUIRE(world.objects.size() == 2);
   REQUIRE(world.objects[1].instance_properties.size() == 1);
   CHECK(world.objects[1].instance_properties[0].key == "ControlZone");
   CHECK(world.objects[1].instance_properties[0].value == "test_object");
}

TEST_CASE("edits delete_entity animation ref object", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .layer_descriptions = {{.name = "[Base]"s}},
      .common_layers = {0},

      .objects =
         {
            world::entities_init,
            std::initializer_list{
               world::object{
                  .name = "test_object"s,

                  .id = world::object_id{0},
               },
            },
         },
      .animation_groups =
         {
            pinned_vector_init{world::max_animation_groups, 256},
            std::initializer_list{
               world::animation_group{
                  .entries =
                     {

                        world::animation_group::entry{"anim0", "other_object"},
                        world::animation_group::entry{"anim1", "test_object"},
                        world::animation_group::entry{"anim2",
                                                      "other_other_object"},

                     },
               },
            },
         },
      .animation_hierarchies =
         {
            pinned_vector_init{world::max_animation_hierarchies, 256},
            std::initializer_list{
               world::animation_hierarchy{
                  .objects = {"other_object", "test_object", "other_other_object"},
               },
               world::animation_hierarchy{
                  .root_object = "test_object",
               },
            },
         },
   };

   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   auto edit = make_delete_entity(world.objects[0].id, world, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.animation_groups[0].entries.size() == 2);
   CHECK(world.animation_groups[0].entries[0].animation == "anim0");
   CHECK(world.animation_groups[0].entries[0].object == "other_object");
   CHECK(world.animation_groups[0].entries[1].animation == "anim2");
   CHECK(world.animation_groups[0].entries[1].object == "other_other_object");

   REQUIRE(world.animation_hierarchies.size() == 1);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 2);
   CHECK(world.animation_hierarchies[0].objects[0] == "other_object");
   CHECK(world.animation_hierarchies[0].objects[1] == "other_other_object");

   edit->revert(edit_context);

   REQUIRE(world.animation_groups[0].entries.size() == 3);
   CHECK(world.animation_groups[0].entries[0].animation == "anim0");
   CHECK(world.animation_groups[0].entries[0].object == "other_object");
   CHECK(world.animation_groups[0].entries[1].animation == "anim1");
   CHECK(world.animation_groups[0].entries[1].object == "test_object");
   CHECK(world.animation_groups[0].entries[2].animation == "anim2");
   CHECK(world.animation_groups[0].entries[2].object == "other_other_object");

   REQUIRE(world.animation_hierarchies.size() == 2);

   REQUIRE(world.animation_hierarchies[0].objects.size() == 3);
   CHECK(world.animation_hierarchies[0].objects[0] == "other_object");
   CHECK(world.animation_hierarchies[0].objects[1] == "test_object");
   CHECK(world.animation_hierarchies[0].objects[2] == "other_other_object");

   CHECK(world.animation_hierarchies[1].root_object == "test_object");
}

TEST_CASE("edits delete_entity object class handle liftime", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   const lowercase_string class_name = world.objects[0].class_name;

   world.objects[0].class_handle = object_class_library.acquire(class_name);

   auto edit = make_delete_entity(world.objects[0].id, world, object_class_library);

   edit->apply(edit_context);

   CHECK(object_class_library.debug_ref_count(class_name) == 0);

   edit->revert(edit_context);

   CHECK(object_class_library.debug_ref_count(class_name) == 1);
}

TEST_CASE("edits delete_entity light gloabl ref", "[Edits]")
{
   world::world world = {
      .name = "Test"s,

      .layer_descriptions = {{.name = "[Base]"s}},
      .common_layers = {0},

      .global_lights =
         {
            .global_light_1 = "sun",
            .global_light_2 = "Sun",
         },

      .lights =
         {
            world::entities_init,
            std::initializer_list{
               world::light{
                  .name = "sun",

                  .id = world::light_id{0},
               },
            },
         },
   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.lights[0].id, world);

   edit->apply(edit_context);

   CHECK(world.global_lights.global_light_1.empty());
   CHECK(world.global_lights.global_light_2.empty());

   edit->revert(edit_context);

   CHECK(world.global_lights.global_light_1 == "sun");
   CHECK(world.global_lights.global_light_2 == "Sun");
}

}
