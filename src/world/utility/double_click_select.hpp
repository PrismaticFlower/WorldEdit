#pragma once

#include "select_common.hpp"

#include "../interaction_context.hpp"
#include "../world.hpp"

#include "math/frustum.hpp"

namespace we::world {

struct blocks_custom_mesh_bvh_library;
struct object_class_library;

/// @brief Add (or remove) similar entities that intersect the frustum to (or from) the selection.
/// @param hovered_entity The hovered entity to use for similarity.
/// @param world The world.
/// @param object_classes The object classes.
/// @param bvh_library The the BVH library for custom blocks.
/// @param frustumWS The frustum.
/// @param op The operation to do (add or remove).
/// @param selection The selection to edit.
void double_click_select(const interaction_target& hovered_entity, const world& world,
                         const object_class_library& object_classes,
                         const blocks_custom_mesh_bvh_library& bvh_library,
                         const frustum& frustumWS, select_op op, selection& selection,
                         const select_settings& settings) noexcept;

}