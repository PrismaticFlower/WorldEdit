#include "pch.h"

#include "world/interaction_context.hpp"

namespace we::world::tests {

namespace {

const we::world::world test_world = {
   .name = "Test",

   .requirements = {{.file_type = "world", .entries = {"Test"}}},

   .layer_descriptions = {{.name = "[Base]"}},
   .game_modes = {{.name = "Common", .layers = {0}}},

   .terrain = {},
   .global_lights = {.env_map_texture = "sky"},

   .objects = {object{.id = object_id{0}}},

   .paths = {path{
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
   }},
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

}
