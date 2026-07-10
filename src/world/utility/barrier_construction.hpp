#pragma once

#include "types.hpp"

#include "../object.hpp"
#include "../object_class_library.hpp"

#include <array>

namespace we::world {

struct barrier_metrics {
   float3 position;
   float2 size;
   float rotation_angle = 0.0f;
};

auto barrier_from_points(const std::array<float3, 8>& points) noexcept -> barrier_metrics;

auto barrier_from_object(const object& object,
                         const object_class_library& object_classes) noexcept
   -> barrier_metrics;

}
