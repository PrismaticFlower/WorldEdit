#pragma once

#include "mesh_vertex.hpp"

#include "types.hpp"

#include <array>
#include <vector>

namespace we::world {

struct block_custom_mesh {
   std::vector<block_vertex> vertices;
   std::vector<std::array<uint16, 3>> triangles;
   std::vector<std::array<uint16, 4>> occluders;

   std::vector<block_collision_vertex> collision_vertices;
   std::vector<std::array<uint16, 3>> collision_triangles;
   std::vector<std::array<uint16, 4>> collision_occluders;

   std::vector<float3> snap_points;
   std::vector<std::array<uint16, 2>> snap_edges;
};

}