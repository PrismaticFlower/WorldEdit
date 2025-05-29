#pragma once

#include "../blocks.hpp"

namespace we::world {

/// @brief Generates the mesh for a stairway.
/// @param stairway The stairway.
/// @return The resulting block_custom_mesh.
auto generate_mesh(const block_custom_mesh_stairway_desc& stairway) noexcept
   -> block_custom_mesh;

}