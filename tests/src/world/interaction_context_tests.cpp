#include "pch.h"

#include "world/interaction_context.hpp"

#include <algorithm>

namespace we::world::tests {

namespace {

const we::world::world test_world = {
   .name = "Test",

   .requirements = {{.file_type = "world", .entries = {"Test"}}},

   .layer_descriptions = {{.name = "[Base]"}},
   .game_modes = {{.name = "Common", .layers = {0}}},

   .terrain = {},
   .global_lights = {.env_map_texture = "sky"},

   .objects = {entities_init, std::initializer_list{object{.id = object_id{0}}}},

   .paths = {entities_init, std::initializer_list{path{
                               .name = "test_path",
                               .layer = 0,

                               .nodes = {{
                                            .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                                            .position = {0.0f, 0.0f, 0.0f},
                                         },
                                         {
                                            .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                                            .position = {1.0f, 1.0f, 1.0f},
                                         }},

                               .id = path_id{3},
                            }}},
};

}

TEST_CASE("world interaction_context is_valid", "[World][ID]")
{
   CHECK(is_valid(object_id{0}, test_world));
   CHECK(not is_valid(object_id{1}, test_world));

   CHECK(is_valid(path_id_node_pair{path_id{3}, 0}, test_world));
   CHECK(is_valid(path_id_node_pair{path_id{3}, 1}, test_world));
   CHECK(not is_valid(path_id_node_pair{path_id{3}, 2}, test_world));
   CHECK(not is_valid(path_id_node_pair{path_id{0}, 0}, test_world));
}

TEST_CASE("world selection", "[World][ID]")
{
   selection selection;

   const auto test_type = [&]<typename T>(const T) {
      const std::size_t start_size = selection.size();

      selection.add(T{0});
      selection.add(T{1});
      selection.add(T{1});

      REQUIRE(selection.size() == start_size + 2);

      CHECK(std::ranges::contains(selection.view(), selected_entity{T{0}}));
      CHECK(std::ranges::contains(selection.view(), selected_entity{T{1}}));
   };

   CHECK(selection.empty());
   CHECK(selection.size() == 0);

   test_type(object_id{});
   test_type(light_id{});
   test_type(region_id{});
   test_type(sector_id{});
   test_type(portal_id{});
   test_type(hintnode_id{});
   test_type(barrier_id{});
   test_type(planning_hub_id{});
   test_type(planning_connection_id{});
   test_type(boundary_id{});

   CHECK(selection[1] == selected_entity{object_id{1}});

   CHECK(not selection.empty());
   CHECK(selection.size() == 20);

   selection.add(path_id_node_pair{path_id{0}, 0});
   selection.add(path_id_node_pair{path_id{0}, 0});
   selection.add(path_id_node_pair{path_id{0}, 1});
   selection.add(path_id_node_pair{path_id{1}, 0});
   selection.add(path_id_node_pair{path_id{1}, 0});
   selection.add(path_id_node_pair{path_id{1}, 1});

   CHECK(selection.size() == 24);

   CHECK(std::ranges::contains(selection.view(),
                               selected_entity{path_id_node_pair{path_id{0}, 0}}));
   CHECK(std::ranges::contains(selection.view(),
                               selected_entity{path_id_node_pair{path_id{0}, 1}}));
   CHECK(std::ranges::contains(selection.view(),
                               selected_entity{path_id_node_pair{path_id{1}, 0}}));
   CHECK(std::ranges::contains(selection.view(),
                               selected_entity{path_id_node_pair{path_id{1}, 1}}));

   CHECK(not selection.empty());

   selection.clear();

   CHECK(selection.empty());
}
}
