#pragma once

#include "select_common.hpp"

#include "../active_elements.hpp"
#include "../interaction_context.hpp"
#include "../world.hpp"

#include "math/frustum.hpp"

namespace we::world {

struct blocks_custom_mesh_bvh_library;
struct object_class_library;

/// @brief Add (or remove) entities that intersect the frustum to (or from) the selection.
/// @param world The world.
/// @param active_entities The entity types to select.
/// @param active_layers The layers to select from.
/// @param object_classes The object classes.
/// @param bvh_library The the BVH library for custom blocks.
/// @param frustumWS The frustum.
/// @param op The operation to do (add or remove).
/// @param selection The selection to edit.
void drag_select(const world& world, const active_entity_types active_entities,
                 const active_layers active_layers,
                 const object_class_library& object_classes,
                 const blocks_custom_mesh_bvh_library& bvh_library,
                 const frustum& frustumWS, select_op op, selection& selection,
                 const select_settings& settings) noexcept;

}