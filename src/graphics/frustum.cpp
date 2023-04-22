
#include "frustum.hpp"
#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <array>

#include <functional>

namespace we::graphics {

namespace {

auto get_plane(std::array<float3, 3> points) noexcept -> float4
{
   const std::array edges{points[1] - points[0], points[2] - points[0]};
   const float3 normal = normalize(cross(edges[0], edges[1]));

   return float4{normal, -dot(normal, points[0])};
}

bool outside_plane(const float4& plane, const float3& point) noexcept
{
   return dot(plane, float4{point, 1.0f}) < 0.0f;
}

bool outside_plane(const float4& plane, const float3& point, const float radius) noexcept
{
   return dot(plane, float4{point, 1.0f}) < -radius;
}

}

frustum::frustum(const float4x4& inv_view_projection_matrix, const float z_min,
                 const float z_max) noexcept
{
   const container::enum_array<float4, frustum_corner> corners_proj =
      container::make_enum_array<float4, frustum_corner>(
         {{frustum_corner::bottom_left_near, {-1.0f, -1.0f, z_min, 1.0f}},
          {frustum_corner::bottom_right_near, {1.0f, -1.0f, z_min, 1.0f}},

          {frustum_corner::top_left_near, {-1.0f, 1.0f, z_min, 1.0f}},
          {frustum_corner::top_right_near, {1.0f, 1.0f, z_min, 1.0f}},

          {frustum_corner::bottom_left_far, {-1.0f, -1.0f, z_max, 1.0f}},
          {frustum_corner::bottom_right_far, {1.0f, -1.0f, z_max, 1.0f}},

          {frustum_corner::top_left_far, {-1.0f, 1.0f, z_max, 1.0f}},
          {frustum_corner::top_right_far, {1.0f, 1.0f, z_max, 1.0f}}});

   for (std::size_t i = 0; i < corners.size(); ++i) {
      const float4 position = inv_view_projection_matrix * corners_proj[i];

      corners[i] = float3{position.x, position.y, position.z} / position.w;
   }

   planes[frustum_planes::near_] =
      get_plane({corners[frustum_corner::top_left_near],
                 corners[frustum_corner::top_right_near],
                 corners[frustum_corner::bottom_left_near]});

   planes[frustum_planes::far_] =
      get_plane({corners[frustum_corner::top_left_far],
                 corners[frustum_corner::bottom_left_far],
                 corners[frustum_corner::top_right_far]});

   planes[frustum_planes::bottom] =
      get_plane({corners[frustum_corner::bottom_left_near],
                 corners[frustum_corner::bottom_right_far],
                 corners[frustum_corner::bottom_left_far]});

   planes[frustum_planes::top] = get_plane(
      {corners[frustum_corner::top_left_near], corners[frustum_corner::top_left_far],
       corners[frustum_corner::top_right_far]});

   planes[frustum_planes::left] =
      get_plane({corners[frustum_corner::top_left_near],
                 corners[frustum_corner::bottom_left_far],
                 corners[frustum_corner::top_left_far]});

   planes[frustum_planes::right] =
      get_plane({corners[frustum_corner::top_right_near],
                 corners[frustum_corner::top_right_far],
                 corners[frustum_corner::bottom_right_far]});
}

frustum::frustum(const float4x4& inv_view_projection_matrix) noexcept
   : frustum{inv_view_projection_matrix, 0.0f, 1.0f}
{
}

bool intersects(const frustum& frustum, const math::bounding_box& bbox)
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

   const auto outside_corner = [&](const std::size_t i, auto comparator,
                                   const float corner) {
      bool outside = true;

      for (const auto& frustum_corner : frustum.corners) {
         outside &= comparator(index(frustum_corner, i), corner);
      }

      return outside;
   };

   if (outside_corner(0, std::greater<>{}, bbox.max.x)) return false;
   if (outside_corner(0, std::less<>{}, bbox.min.x)) return false;
   if (outside_corner(1, std::greater<>{}, bbox.max.y)) return false;
   if (outside_corner(1, std::less<>{}, bbox.min.y)) return false;
   if (outside_corner(2, std::greater<>{}, bbox.max.z)) return false;
   if (outside_corner(2, std::less<>{}, bbox.min.z)) return false;

   return true;
}

bool intersects_shadow_cascade(const frustum& frustum, const math::bounding_box& bbox)
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

bool intersects(const frustum& frustum, const float3& position, const float radius)
{
   for (const auto& plane : frustum.planes) {
      if (outside_plane(plane, position, radius)) {
         return false;
      }
   }

   return true;
}
}
