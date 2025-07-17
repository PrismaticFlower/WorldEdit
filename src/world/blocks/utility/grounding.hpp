#pragma once

#include "../../active_elements.hpp"
#include "../../object_class_library.hpp"
#include "../../world.hpp"

#include <optional>

namespace we::world {

struct blocks_custom_mesh_bvh_library;

/// @brief Get the grounded position of a block. Or nullopt if the object is already grounded or can't be grounded.
/// @param id The id of the block.
/// @param block_index The index of the block.
/// @param world The world.
/// @param object_classes The object classes to use.
/// @param blocks_bvh_library The BVH library to use custom mesh blocks.
/// @param active_layers The active layers to raycast against.
/// @return The grounded position of the block or nullopt.
auto ground_block(const block_id id, const uint32 block_index,
                  const world& world, const object_class_library& object_classes,
                  const blocks_custom_mesh_bvh_library& blocks_bvh_library,
                  const active_layers active_layers) noexcept -> std::optional<float3>;

}