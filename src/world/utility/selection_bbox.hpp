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

}