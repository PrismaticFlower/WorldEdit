#pragma once

#include "mesh.hpp"

#include <span>
#include <vector>

namespace we::world {

auto cull_hidden_triangles(std::span<const block_world_triangle> triangles,
                           std::span<const block_world_occluder> occluders) noexcept
   -> std::vector<block_world_triangle>;

}