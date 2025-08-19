#pragma once

#include "../../blocks.hpp"
#include "../../interaction_context.hpp"

#include "mesh_scenes.hpp"

#include <vector>

namespace we::world {

auto gather_export_meshes(const blocks& blocks) -> std::vector<block_export_scene>;

auto gather_export_meshes(const blocks& blocks, const selection& selection)
   -> std::vector<block_export_scene>;

}
