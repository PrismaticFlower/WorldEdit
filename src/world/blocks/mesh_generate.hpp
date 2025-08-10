#pragma once

#include "custom_mesh.hpp"
#include "custom_mesh_description.hpp"

namespace we::world {

/// @brief Generates the mesh for a stairway.
/// @param stairway The stairway.
/// @return The resulting block_custom_mesh.
auto generate_mesh(const block_custom_mesh_description_stairway& stairway) noexcept
   -> block_custom_mesh;

/// @brief Generates the mesh for a ring.
/// @param ring The ring.
/// @return The resulting block_custom_mesh.
auto generate_mesh(const block_custom_mesh_description_ring& ring) noexcept
   -> block_custom_mesh;

/// @brief Generates the mesh for a beveled box.
/// @param box The beveled box.
/// @return The resulting block_custom_mesh.
auto generate_mesh(const block_custom_mesh_description_beveled_box& box) noexcept
   -> block_custom_mesh;

/// @brief Generates the mesh for a cubic curve.
/// @param curve The curve.
/// @return The resulting block_custom_mesh.
auto generate_mesh(const block_custom_mesh_description_curve& curve) noexcept
   -> block_custom_mesh;

/// @brief Generates the mesh for a cylinder.
/// @param cylinder The cylinder.
/// @return The resulting block_custom_mesh.
auto generate_mesh(const block_custom_mesh_description_cylinder& cylinder) noexcept
   -> block_custom_mesh;

/// @brief Generates the mesh for a cone.
/// @param cone The cone.
/// @return The resulting block_custom_mesh.
auto generate_mesh(const block_custom_mesh_description_cone& cone) noexcept
   -> block_custom_mesh;

/// @brief Generates the mesh for a arch.
/// @param arch The arch.
/// @return The resulting block_custom_mesh.
auto generate_mesh(const block_custom_mesh_description_arch& arch) noexcept
   -> block_custom_mesh;
}