#pragma once

#include "../blocks.hpp"

namespace we::world {

/// @brief Generate the mesh for a stairway.
/// @param stairway The stairway.
/// @param out The output block_custom_mesh. Allocations can be reused if their size matches the needed size.
void generate_mesh(const block_description_stairway& stairway,
                   block_custom_mesh& out) noexcept;

/// @brief Generates the mesh for a stairway.
/// @param stairway The stairway.
/// @return The tesulting block_custom_mesh.
auto generate_mesh(const block_description_stairway& stairway) noexcept
   -> block_custom_mesh;

}