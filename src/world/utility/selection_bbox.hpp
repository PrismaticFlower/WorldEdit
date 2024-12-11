#pragma once

#include "../interaction_context.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"
#include "math/bounding_box.hpp"

#include <span>

namespace we::world {

/// @brief Return a bounding box for the selection. To enable use with focus on and orbit camera the bounds for entities like hintnodes approximately match their visualizer rather than being a single point.
/// @param world The world.
/// @param selection The selection.
/// @param object_classes The object class library.
/// @param path_node_size The size of path nodes.
/// @return The bounding box.
auto selection_bbox_for_camera(const world& world,
                               const std::span<const selected_entity> selection,
                               const object_class_library& object_classes,
                               const float path_node_size) -> math::bounding_box;

struct selection_metrics {
   math::bounding_box bboxWS;
   float3 centreWS;
};

/// @brief Return a bounding box for the selection. For use with selection move tools. AI planning connections do not contribute to the bounding box.
/// @param world The world.
/// @param selection The selection.
/// @param object_classes The object class library.
/// @return The bounding box.
auto selection_metrics_for_move(const world& world,
                                const std::span<const selected_entity> selection,
                                const object_class_library& object_classes)
   -> selection_metrics;

}