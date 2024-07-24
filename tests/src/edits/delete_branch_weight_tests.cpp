#include "pch.h"

#include "edits/delete_branch_weight.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits delete_branch_weight", "[Edits]")
{
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
                                               world::planning_branch_weights{
                                                  .hub_index = 3,
                                                  .connection_index = 2,
                                                  .soldier = 25.0f,
                                               },
                                               world::planning_branch_weights{
                                                  .hub_index = 0,
                                                  .connection_index = 2,
                                                  .soldier = 75.0f,
                                               },
                                               world::planning_branch_weights{
                                                  .hub_index = 0,
                                                  .connection_index = 3,
                                                  .soldier = 75.0f,
                                               },
                                               world::planning_branch_weights{
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

   const float2 point = {1e10f, 0.0f};
   const world::position_key key{.time = 4.0f, .position = {4.0f, 4.0f, 4.0f}};

   auto edit = make_delete_branch_weight(&world.planning_hubs[1].weights, 1);

   edit->apply(edit_context);

   REQUIRE(world.planning_hubs[1].weights.size() == 3);
   CHECK(world.planning_hubs[1].weights[0] == world::planning_branch_weights{
                                                 .hub_index = 3,
                                                 .connection_index = 2,
                                                 .soldier = 25.0f,
                                              });
   CHECK(world.planning_hubs[1].weights[1] == world::planning_branch_weights{
                                                 .hub_index = 0,
                                                 .connection_index = 3,
                                                 .soldier = 75.0f,
                                              });
   CHECK(world.planning_hubs[1].weights[2] == world::planning_branch_weights{
                                                 .hub_index = 1,
                                                 .connection_index = 4,
                                                 .soldier = 50.0f,
                                              });

   edit->revert(edit_context);

   REQUIRE(world.planning_hubs[1].weights.size() == 4);
   CHECK(world.planning_hubs[1].weights[0] == world::planning_branch_weights{
                                                 .hub_index = 3,
                                                 .connection_index = 2,
                                                 .soldier = 25.0f,
                                              });
   CHECK(world.planning_hubs[1].weights[1] == world::planning_branch_weights{
                                                 .hub_index = 0,
                                                 .connection_index = 2,
                                                 .soldier = 75.0f,
                                              });
   CHECK(world.planning_hubs[1].weights[2] == world::planning_branch_weights{
                                                 .hub_index = 0,
                                                 .connection_index = 3,
                                                 .soldier = 75.0f,
                                              });
   CHECK(world.planning_hubs[1].weights[3] == world::planning_branch_weights{
                                                 .hub_index = 1,
                                                 .connection_index = 4,
                                                 .soldier = 50.0f,
                                              });
}
}
