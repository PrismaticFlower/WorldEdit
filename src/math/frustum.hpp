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

   count
};

enum class frustum_planes {
   near_,
   far_,
   top,
   bottom,
   left,
   right,

   count
};

struct frustum {
   frustum(const float4x4& inv_view_projection_matrix, const float3 ndc_min,
           const float3 ndc_max) noexcept;

   frustum(const float4x4& inv_view_projection_matrix, const float z_min,
           const float z_max) noexcept;

   explicit frustum(const float4x4& inv_view_projection_matrix) noexcept;

   container::enum_array<float3, frustum_corner> corners;
   container::enum_array<float4, frustum_planes> planes;
};

bool intersects(const frustum& frustum, const math::bounding_box& bbox) noexcept;

bool intersects_shadow_cascade(const frustum& frustum,
                               const math::bounding_box& bbox) noexcept;

bool intersects(const frustum& frustum, const float3& position,
                const float radius) noexcept;

auto transform(const frustum& world_frustum, const quaternion& rotation,
               const float3& position) noexcept -> frustum;

}
