
#include "frustrum.hpp"
#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <array>

#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>

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

frustrum::frustrum(const float4x4& inv_view_projection_matrix) noexcept
{
   constexpr container::enum_array<float4, frustrum_corner> corners_proj =
      container::make_enum_array<float4, frustrum_corner>(
         {{frustrum_corner::bottom_left_near, {-1.0f, -1.0f, 0.0f, 1.0f}},
          {frustrum_corner::bottom_right_near, {1.0f, -1.0f, 0.0f, 1.0f}},

          {frustrum_corner::top_left_near, {-1.0f, 1.0f, 0.0f, 1.0f}},
          {frustrum_corner::top_right_near, {1.0f, 1.0f, 0.0f, 1.0f}},

          {frustrum_corner::bottom_left_far, {-1.0f, -1.0f, 1.0f, 1.0f}},
          {frustrum_corner::bottom_right_far, {1.0f, -1.0f, 1.0f, 1.0f}},

          {frustrum_corner::top_left_far, {-1.0f, 1.0f, 1.0f, 1.0f}},
          {frustrum_corner::top_right_far, {1.0f, 1.0f, 1.0f, 1.0f}}});

   ranges::copy(
      corners_proj | ranges::views::transform([&](float4 position) {
         position = inv_view_projection_matrix * position;

         return float3{position.x, position.y, position.z} / position.w;
      }),
      corners.begin());

   planes[frustrum_planes::near_] =
      get_plane({corners[frustrum_corner::top_left_near],
                 corners[frustrum_corner::top_right_near],
                 corners[frustrum_corner::bottom_left_near]});

   planes[frustrum_planes::far_] =
      get_plane({corners[frustrum_corner::top_left_far],
                 corners[frustrum_corner::bottom_left_far],
                 corners[frustrum_corner::top_right_far]});

   planes[frustrum_planes::bottom] =
      get_plane({corners[frustrum_corner::bottom_left_near],
                 corners[frustrum_corner::bottom_right_far],
                 corners[frustrum_corner::bottom_left_far]});

   planes[frustrum_planes::top] =
      get_plane({corners[frustrum_corner::top_left_near],
                 corners[frustrum_corner::top_left_far],
                 corners[frustrum_corner::top_right_far]});

   planes[frustrum_planes::left] =
      get_plane({corners[frustrum_corner::top_left_near],
                 corners[frustrum_corner::bottom_left_far],
                 corners[frustrum_corner::top_left_far]});

   planes[frustrum_planes::right] =
      get_plane({corners[frustrum_corner::top_right_near],
                 corners[frustrum_corner::top_right_far],
                 corners[frustrum_corner::bottom_right_far]});
}

bool intersects(const frustrum& frustrum, const math::bounding_box& bbox)
{
   for (const auto& plane : frustrum.planes) {
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

      for (const auto& frustrum_corner : frustrum.corners) {
         outside &= comparator(index(frustrum_corner, i), corner);
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

bool intersects_shadow_cascade(const frustrum& frustrum, const math::bounding_box& bbox)
{
   for (auto& plane : frustrum.planes | ranges::views::drop(1)) {
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

bool intersects(const frustrum& frustrum, const float3& position, const float radius)
{
   for (const auto& plane : frustrum.planes) {
      if (outside_plane(plane, position, radius)) {
         return false;
      }
   }

   return true;
}
}
