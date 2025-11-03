#pragma once

#include "../../active_elements.hpp"
#include "../../blocks.hpp"
#include "../../interaction_context.hpp"

#include "math/frustum.hpp"

namespace we::world {

struct blocks_custom_mesh_bvh_library;

enum class block_drag_select_op { add, remove };

/// @brief Add (or remove) blocks that intersect the frustum to (or from) the selection.
/// @param blocks The world blocks.
/// @param active_layers The layers to select from.
/// @param bvh_library The the BVH library for custom blocks.
/// @param frustumWS The frustum.
/// @param op The operation to do (add or remove).
/// @param selection The selection to edit.
void drag_select(const blocks& blocks, const active_layers active_layers,
                 const blocks_custom_mesh_bvh_library& bvh_library,
                 const frustum& frustumWS, block_drag_select_op op,
                 selection& selection) noexcept;

}