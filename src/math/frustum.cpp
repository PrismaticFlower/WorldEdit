
#include "frustum.hpp"
#include "matrix_funcs.hpp"
#include "plane_funcs.hpp"
#include "quaternion_funcs.hpp"
#include "vector_funcs.hpp"

#include <array>

#include <functional>

namespace we {

namespace {

bool outside_plane(const float4& plane, const float3& point) noexcept
{
   return dot(plane, float4{point, 1.0f}) < 0.0f;
}

bool outside_plane(const float4& plane, const float3& point, const float radius) noexcept
{
   return dot(plane, float4{point, 1.0f}) < -radius;
}

}

frustum::frustum(const float4x4& inv_view_projection_matrix,
                 const float3 ndc_min, const float3 ndc_max) noexcept
{
   const container::enum_array<float4, frustum_corner> corners_proj =
      container::make_enum_array<float4, frustum_corner>(
         {{frustum_corner::bottom_left_near, {ndc_min.x, ndc_min.y, ndc_max.z, 1.0f}},
          {frustum_corner::bottom_right_near, {ndc_max.x, ndc_min.y, ndc_max.z, 1.0f}},

          {frustum_corner::top_left_near, {ndc_min.x, ndc_max.y, ndc_max.z, 1.0f}},
          {frustum_corner::top_right_near, {ndc_max.x, ndc_max.y, ndc_max.z, 1.0f}},

          {frustum_corner::bottom_left_far, {ndc_min.x, ndc_min.y, ndc_min.z, 1.0f}},
          {frustum_corner::bottom_right_far, {ndc_max.x, ndc_min.y, ndc_min.z, 1.0f}},

          {frustum_corner::top_left_far, {ndc_min.x, ndc_max.y, ndc_min.z, 1.0f}},
          {frustum_corner::top_right_far, {ndc_max.x, ndc_max.y, ndc_min.z, 1.0f}}});

   for (std::size_t i = 0; i < corners.size(); ++i) {
      const float4 position = inv_view_projection_matrix * corners_proj[i];

      corners[i] = float3{position.x, position.y, position.z} / position.w;
   }

   planes[frustum_planes::near_] =
      make_plane(corners[frustum_corner::top_left_near],
                 corners[frustum_corner::top_right_near],
                 corners[frustum_corner::bottom_left_near]);

   planes[frustum_planes::far_] =
      make_plane(corners[frustum_corner::top_left_far],
                 corners[frustum_corner::bottom_left_far],
                 corners[frustum_corner::top_right_far]);

   planes[frustum_planes::bottom] =
      make_plane(corners[frustum_corner::bottom_left_near],
                 corners[frustum_corner::bottom_right_far],
                 corners[frustum_corner::bottom_left_far]);

   planes[frustum_planes::top] =
      make_plane(corners[frustum_corner::top_left_near],
                 corners[frustum_corner::top_left_far],
                 corners[frustum_corner::top_right_far]);

   planes[frustum_planes::left] =
      make_plane(corners[frustum_corner::top_left_near],
                 corners[frustum_corner::bottom_left_far],
                 corners[frustum_corner::top_left_far]);

   planes[frustum_planes::right] =
      make_plane(corners[frustum_corner::top_right_near],
                 corners[frustum_corner::top_right_far],
                 corners[frustum_corner::bottom_right_far]);
}

frustum::frustum(const float4x4& inv_view_projection_matrix, const float z_min,
                 const float z_max) noexcept
   : frustum{inv_view_projection_matrix, {-1.0f, -1.0f, z_min}, {1.0f, 1.0f, z_max}}
{
}

frustum::frustum(const float4x4& inv_view_projection_matrix) noexcept
   : frustum{inv_view_projection_matrix, 0.0f, 1.0f}
{
}

bool intersects(const frustum& frustum, const math::bounding_box& bbox) noexcept
{
   for (const auto& plane : frustum.planes) {
      if (outside_plane(plane, {bbox.min.x, bbox.min.y, bbox.min.z}) &
          outside_plane(plane, {bbox.max.x, bbox.min.y, bbox.min.z}) &
          outside_plane(plane, {bbox.min.x, bbox.max.y, bbox.min.z}) &
          outside_plane(plane, {bbox.max.x, bbox.max.y, bbox.min.z}) &
          outside_plane(plane, {bbox.min.x, bbox.min.y, bbox.max.z}) &
          outside_plane(plane, {bbox.max.x, bbox.min.y, bbox.max.z}) &
          outside_plane(plane, {bbox.min.x, bbox.max.y, bbox.max.z}) &
          outside_plane(plane, {bbox.max.x, bbox.max.y, bbox.max.z})) {
         return false;
      }
   }

   const auto outside_corner = [&](const float float3::*axis, auto comparator,
                                   const float corner) {
      bool outside = true;

      for (const auto& frustum_corner : frustum.corners) {
         outside &= comparator(frustum_corner.*axis, corner);
      }

      return outside;
   };

   if (outside_corner(&float3::x, std::greater<>{}, bbox.max.x)) return false;
   if (outside_corner(&float3::x, std::less<>{}, bbox.min.x)) return false;
   if (outside_corner(&float3::y, std::greater<>{}, bbox.max.y)) return false;
   if (outside_corner(&float3::y, std::less<>{}, bbox.min.y)) return false;
   if (outside_corner(&float3::z, std::greater<>{}, bbox.max.z)) return false;
   if (outside_corner(&float3::z, std::less<>{}, bbox.min.z)) return false;

   return true;
}

bool intersects_shadow_cascade(const frustum& frustum, const math::bounding_box& bbox) noexcept
{
   for (std::size_t i = 0; i < (frustum.planes.size() - 1); ++i) {
      const float4 plane = frustum.planes[i];

      if (outside_plane(plane, {bbox.min.x, bbox.min.y, bbox.min.z}) &
          outside_plane(plane, {bbox.max.x, bbox.min.y, bbox.min.z}) &
          outside_plane(plane, {bbox.min.x, bbox.max.y, bbox.min.z}) &
          outside_plane(plane, {bbox.max.x, bbox.max.y, bbox.min.z}) &
          outside_plane(plane, {bbox.min.x, bbox.min.y, bbox.max.z}) &
          outside_plane(plane, {bbox.max.x, bbox.min.y, bbox.max.z}) &
          outside_plane(plane, {bbox.min.x, bbox.max.y, bbox.max.z}) &
          outside_plane(plane, {bbox.max.x, bbox.max.y, bbox.max.z})) {
         return false;
      }
   }

   return true;
}

bool intersects(const frustum& frustum, const float3& position, const float radius) noexcept
{
   for (const auto& plane : frustum.planes) {
      if (outside_plane(plane, position, radius)) {
         return false;
      }
   }

   return true;
}

auto transform(const frustum& world_frustum, const quaternion& rotation,
               const float3& position) noexcept -> frustum
{
   frustum frustum = world_frustum;

   for (float3& point : frustum.corners) {
      point = rotation * point + position;
   }

   frustum.planes[frustum_planes::near_] =
      make_plane(frustum.corners[frustum_corner::top_left_near],
                 frustum.corners[frustum_corner::top_right_near],
                 frustum.corners[frustum_corner::bottom_left_near]);

   frustum.planes[frustum_planes::far_] =
      make_plane(frustum.corners[frustum_corner::top_left_far],
                 frustum.corners[frustum_corner::bottom_left_far],
                 frustum.corners[frustum_corner::top_right_far]);

   frustum.planes[frustum_planes::bottom] =
      make_plane(frustum.corners[frustum_corner::bottom_left_near],
                 frustum.corners[frustum_corner::bottom_right_far],
                 frustum.corners[frustum_corner::bottom_left_far]);

   frustum.planes[frustum_planes::top] =
      make_plane(frustum.corners[frustum_corner::top_left_near],
                 frustum.corners[frustum_corner::top_left_far],
                 frustum.corners[frustum_corner::top_right_far]);

   frustum.planes[frustum_planes::left] =
      make_plane(frustum.corners[frustum_corner::top_left_near],
                 frustum.corners[frustum_corner::bottom_left_far],
                 frustum.corners[frustum_corner::top_left_far]);

   frustum.planes[frustum_planes::right] =
      make_plane(frustum.corners[frustum_corner::top_right_near],
                 frustum.corners[frustum_corner::top_right_far],
                 frustum.corners[frustum_corner::bottom_right_far]);

   return frustum;
}

}
