#pragma once

#include "../object_class.hpp"
#include "../world.hpp"

#include <optional>
#include <span>

#include <absl/container/flat_hash_map.h>

namespace we::world {

auto get_snapped_position(
   const object& snapping_object, const float3 snapping_position,
   const std::span<const object> world_objects, const float snap_radius,
   const absl::flat_hash_map<lowercase_string, object_class>& object_classes)
   -> std::optional<float3>;

}
