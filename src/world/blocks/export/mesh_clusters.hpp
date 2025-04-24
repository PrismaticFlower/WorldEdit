#pragma once

#include "types.hpp"

#include "mesh.hpp"

#include <span>
#include <vector>

namespace we::world {

auto build_mesh_clusters(std::span<const block_world_triangle> triangles,
                         int32 min_triangles, int32 max_subdivisions) noexcept
   -> std::vector<std::vector<uint32>>;

}