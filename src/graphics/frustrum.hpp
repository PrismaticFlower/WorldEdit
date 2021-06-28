#pragma once

#include "camera.hpp"
#include "container/enum_array.hpp"
#include "math/bounding_box.hpp"
#include "types.hpp"

namespace we::graphics {

enum class frustrum_corner {
   bottom_left_near,
   bottom_right_near,
   top_left_near,
   top_right_near,

   bottom_left_far,
   bottom_right_far,
   top_left_far,
   top_right_far,

   count
};

enum class frustrum_planes {
   near_,
   far_,
   top,
   bottom,
   left,
   right,

   count
};

struct frustrum {
   explicit frustrum(const camera& camera) noexcept;

   container::enum_array<float3, frustrum_corner> corners;
   container::enum_array<float4, frustrum_planes> planes;
};

bool intersects(const frustrum& frustrum, const math::bounding_box& bbox);

bool intersects_shadow_cascade(const frustrum& frustrum, const math::bounding_box& bbox);

bool intersects(const frustrum& frustrum, const float3& position, const float radius);

}
