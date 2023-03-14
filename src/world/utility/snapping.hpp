#pragma once

#include "../object_class_library.hpp"
#include "../world.hpp"

#include <optional>
#include <span>

namespace we::world {

auto get_snapped_position(const object& snapping_object, const float3 snapping_position,
                          const std::span<const object> world_objects,
                          const float snap_radius,
                          const object_class_library& object_classes)
   -> std::optional<float3>;

auto get_snapped_position(const float3 snapping_position,
                          const std::span<const object> world_objects,
                          const float snap_radius,
                          const object_class_library& object_classes)
   -> std::optional<float3>;

}
