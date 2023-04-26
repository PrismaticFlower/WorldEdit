#pragma once

#include "types.hpp"

#include <string>
#include <vector>

namespace we::assets::sky {

struct dome_model_rotation {
   float speed = 0.0f; // Rotations per second
   float3 direction = {0.0f, 1.0f, 0.0f};
};

struct dome_model {
   std::string geometry;
   float offset = 0.0f;
   float movement_scale = 1.0f; // Camera Height x Movement Scale
   dome_model_rotation rotation;
};

struct config {
   std::string terrain_normal_map;
   float terrain_normal_map_tiling = 1.0f;
   std::vector<dome_model> dome_models;
};

}
