#pragma once

#include "model.hpp"

#include <span>

namespace we::munge {

struct collision_mesh_input {
   std::string node_name;

   collision_flags mask = collision_flags::all;

   bool do_not_simplify = false;

   std::vector<float3> vertices;
   std::vector<std::array<uint16, 3>> triangles;
};

auto build_collision_mesh(collision_mesh_input input) -> collision_mesh;

}