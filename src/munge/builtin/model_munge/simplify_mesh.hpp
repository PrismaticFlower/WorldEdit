#pragma once

#include "types.hpp"

#include <array>
#include <memory>
#include <span>
#include <vector>

namespace we::munge {

struct simplified_mesh_face {
   uint32 index_begin = 0;
   uint8 index_count = 0;
};

struct simplified_collision_mesh {
   std::vector<float3> vertices;
   std::vector<uint16> face_indices;
   std::vector<simplified_mesh_face> faces;
};

auto simplify_collision_mesh(std::span<const float3> vertices,
                             std::span<const std::array<uint16, 3>> triangles)
   -> simplified_collision_mesh;

}
