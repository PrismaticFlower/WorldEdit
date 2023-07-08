#include "pch.h"

#include "edits/delete_path_property.hpp"
#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

const we::world::world delete_path_property_test_world = {
   .paths =
      {
         {
            .properties =
               {
                  {"Key", "Value"},
                  {"Cake", "Mm"},
                  {"MMm", "Cake"},
               },
            .nodes =
               {
                  {
                     .properties =
                        {
                           {"Key", "Value"},
                        },
                  },

                  {
                     .properties =
                        {
                           {"Sunflower", "Pretty"},
                           {"Key", "Value"},
                           {"OtherFlowers", "AlsoPretty"},
                        },
                  },

                  {
                     .properties =
                        {
                           {"Key", "Value"},
                        },
                  },
               },

            .id = world::path_id{0},
         },
      },
};

}

TEST_CASE("edits delete_path_property", "[Edits]")
{
   world::world world = delete_path_property_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_delete_path_property(world::path_id{0}, 1, world);

   using property = world::path::property;

   action->apply(edit_context);

   REQUIRE(world.paths[0].properties.size() == 2);
   CHECK(world.paths[0].properties[0] == property{"Key", "Value"});
   CHECK(world.paths[0].properties[1] == property{"MMm", "Cake"});

   action->revert(edit_context);

   REQUIRE(world.paths == delete_path_property_test_world.paths);
}

TEST_CASE("edits make_delete_path_node_property", "[Edits]")
{
   world::world world = delete_path_property_test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto action = make_delete_path_node_property(world::path_id{0}, 1, 1, world);

   using property = world::path::property;

   action->apply(edit_context);

   REQUIRE(world.paths[0].nodes[1].properties.size() == 2);
   CHECK(world.paths[0].nodes[1].properties[0] ==
         property{"Sunflower", "Pretty"});
   CHECK(world.paths[0].nodes[1].properties[1] ==
         property{"OtherFlowers", "AlsoPretty"});

   action->revert(edit_context);

   REQUIRE(world.paths[0].nodes == delete_path_property_test_world.paths[0].nodes);
}

}
