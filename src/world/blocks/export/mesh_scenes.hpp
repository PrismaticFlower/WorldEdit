#pragma once

#include "../../blocks.hpp"

#include "assets/msh/scene.hpp"

#include "mesh.hpp"

#include <span>

namespace we::world {

struct block_export_scene {
   block_foley_group foley_group = block_foley_group::stone;
   assets::msh::scene msh_scene;
};

auto build_mesh_scenes(std::span<const block_world_triangle> triangles,
                       std::span<const std::vector<uint32>> triangle_clusters,
                       std::span<const block_material> materials) noexcept
   -> std::vector<block_export_scene>;

}
