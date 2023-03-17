#include "pch.h"

#include "edits/delete_entity.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits delete_entity object", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.objects[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.objects.empty());

   REQUIRE(world.paths[0].properties.size() == 1);

   REQUIRE(world.sectors[0].objects.empty());

   REQUIRE(world.hintnodes[0].command_post == "");

   edit->revert(edit_context);

   REQUIRE(world.objects.size() == 1);
   REQUIRE(world.objects[0] == test_world.objects[0]);

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

   auto delete_node_0 = make_delete_entity({world.paths[0].id, 0}, world);

   delete_node_0->apply(edit_context);

   REQUIRE(not world.paths.empty());
   REQUIRE(world.paths[0].nodes.size() == 1);
   REQUIRE(world.paths[0].nodes[0] == test_world.paths[0].nodes[1]);

   REQUIRE(world.objects[0].instance_properties.size() == 9);

   auto delete_node_1 = make_delete_entity({world.paths[0].id, 1}, world);

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

   auto edit = make_delete_entity(world.planning_hubs[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.planning_hubs.size() == 1);
   REQUIRE(world.planning_hubs[0] == test_world.planning_hubs[1]);

   REQUIRE(world.planning_connections.empty());

   REQUIRE(not world.planning_hub_index.contains(test_world.planning_hubs[0].id));
   REQUIRE(world.planning_hub_index.at(test_world.planning_hubs[1].id) == 0);

   edit->revert(edit_context);

   REQUIRE(world.planning_hubs.size() == 2);
   REQUIRE(world.planning_hubs[0] == test_world.planning_hubs[0]);
   REQUIRE(world.planning_hubs[1] == test_world.planning_hubs[1]);

   REQUIRE(world.planning_connections.size() == 1);
   REQUIRE(world.planning_connections[0] == test_world.planning_connections[0]);

   REQUIRE(world.planning_hub_index.at(test_world.planning_hubs[0].id) == 0);
   REQUIRE(world.planning_hub_index.at(test_world.planning_hubs[1].id) == 1);
}

TEST_CASE("edits delete_entity planning_connection", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_delete_entity(world.planning_connections[0].id, world);

   edit->apply(edit_context);

   REQUIRE(world.planning_connections.empty());

   edit->revert(edit_context);

   REQUIRE(world.planning_connections.size() == 1);
   REQUIRE(world.planning_connections[0] == test_world.planning_connections[0]);
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

}
