#pragma once

#include "../active_elements.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include <optional>
#include <span>

namespace we::world {

struct snapping_entity {
   quaternion rotation;
   float3 positionWS;
   math::bounding_box bboxOS;
};

struct snapping_flags {
   bool snap_to_surfaces : 1 = true;
   bool snap_to_corners : 1 = true;
   bool snap_to_edge_midpoints : 1 = true;
   bool snap_to_face_midpoints : 1 = true;
};

auto get_snapped_position(const snapping_entity& snapping,
                          const std::span<const object> world_objects,
                          const float snap_radius, const snapping_flags flags,
                          const active_layers active_layers,
                          const object_class_library& object_classes) noexcept -> float3;

auto get_snapped_position(const object& snapping_object, const float3 snapping_positionWS,
                          const std::span<const object> world_objects,
                          const float snap_radius, const snapping_flags flags,
                          const active_layers active_layers,
                          const object_class_library& object_classes) noexcept -> float3;

auto get_snapped_position(const float3 snapping_position,
                          const std::span<const object> world_objects,
                          const float snap_radius,
                          const object_class_library& object_classes)
   -> std::optional<float3>;

}
