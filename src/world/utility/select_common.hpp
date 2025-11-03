#pragma once

namespace we::world {

enum class select_op { add, remove };

struct select_settings {
   float path_node_radius = 0.0f;
   float barrier_visualizer_height = 0.0f;
   float hub_visualizer_height = 0.0f;
   float connection_visualizer_height = 0.0f;
   float boundary_visualizer_height = 0.0f;
};

}