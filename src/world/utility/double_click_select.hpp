#pragma once

#include "../interaction_context.hpp"
#include "../world.hpp"

#include "math/frustum.hpp"

namespace we::world {

struct blocks_custom_mesh_bvh_library;
struct object_class_library;

enum class double_click_select_op { add, remove };

struct double_click_settings {
   float path_node_radius = 0.0f;
   float barrier_visualizer_height = 0.0f;
   float hub_visualizer_height = 0.0f;
   float connection_visualizer_height = 0.0f;
   float boundary_visualizer_height = 0.0f;
};

/// @brief Add (or remove) blocks that intersect the frustum to (or from) the selection.
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
                         const frustum& frustumWS, double_click_select_op op,
                         selection& selection,
                         const double_click_settings& settings) noexcept;

}