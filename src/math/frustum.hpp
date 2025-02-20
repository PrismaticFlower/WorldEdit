#pragma once

#include "bounding_box.hpp"
#include "container/enum_array.hpp"
#include "types.hpp"

namespace we {

enum class frustum_corner {
   bottom_left_near,
   bottom_right_near,
   top_left_near,
   top_right_near,

   bottom_left_far,
   bottom_right_far,
   top_left_far,
   top_right_far,

   COUNT
};

enum class frustum_planes {
   near_,
   far_,
   top,
   bottom,
   left,
   right,

   COUNT
};

struct frustum {
   frustum(const float4x4& world_from_projection, const float3 ndc_min,
           const float3 ndc_max) noexcept;

   frustum(const float4x4& world_from_projection, const float z_min,
           const float z_max) noexcept;

   explicit frustum(const float4x4& world_from_projection) noexcept;

   container::enum_array<float3, frustum_corner> corners;
   container::enum_array<float4, frustum_planes> planes;
};

bool intersects(const frustum& frustum, const math::bounding_box& bbox) noexcept;

bool intersects_shadow_cascade(const frustum& frustum,
                               const math::bounding_box& bbox) noexcept;

bool intersects(const frustum& frustum, const float3& position,
                const float radius) noexcept;

bool intersects(const frustum& frustum, const float3& v0, const float3& v1,
                const float3& v2) noexcept;

auto transform(const frustum& world_frustum, const quaternion& rotation,
               const float3& position) noexcept -> frustum;

}
