#include "barrier_construction.hpp"

#include "../object_class.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <algorithm>

namespace we::world {

auto barrier_from_points(const std::array<float3, 8>& points) noexcept -> barrier_metrics
{
   // Calculate the convex hull using Graham's scan (https://en.wikipedia.org/wiki/Graham_scan) with insight for sorting using
   // the Z sign of the cross product from https://strncat.github.io/jekyll/update/2023/09/03/convex-hull-graham-scan.html

   std::array<float2, 8> points_2d;

   for (int i = 0; i < 8; ++i) points_2d[i] = {points[i].x, points[i].z};

   float2 p0 = points_2d[0];

   for (int i = 1; i < 8; ++i) {
      if (points_2d[i].y <= p0.y and points_2d[i].x <= p0.x) {
         p0 = points_2d[i];
      }
   }

   std::sort(points_2d.begin(), points_2d.end(), [&](const float2& l, const float2& r) {
      const float sign = cross2d(l - p0, r - p0);

      if (sign > 0.0f) return true;
      if (sign == 0.0f) return distance(p0, l) < distance(p0, r);

      return false;
   });

   std::array<float2, 8> convex_hull = {};
   int convex_hull_size = 0;

   for (const float2& point : points_2d) {
      while (convex_hull_size > 1 and
             cross2d(convex_hull[convex_hull_size - 1] - convex_hull[convex_hull_size - 2],
                     point - convex_hull[convex_hull_size - 2]) <= 0.0f) {
         convex_hull_size -= 1;
      }

      convex_hull[convex_hull_size] = point;
      convex_hull_size += 1;
   }

   // Exhaustive Search Algorithm for a Minimum-Area Rectangle from:
   // Eberly, D. (2015), "Minimum-area rectangle containing a set of points", Geometric Tools, LLC.
   //
   // Our common case is four points and our worst case is 7 points. While this is O(n²) that ends up being O(49) in the
   // worst case for a small, tight loop over a tiny data set. Ship it.

   float2 centre;
   std::array<float2, 2> axes;
   float2 extents;
   float min_area = FLT_MAX;

   for (int i0 = convex_hull_size - 1, i1 = 0; i1 < convex_hull_size; i0 = i1++) {
      const float2 origin = convex_hull[i0];
      const float2 u0 = normalize(convex_hull[i1] - origin);
      const float2 u1 = {-u0.y, u0.x};

      float min0 = 0.0f;
      float max0 = 0.0f;
      float min1 = 0.0f;
      float max1 = 0.0f;

      for (int j = 0; j < convex_hull_size; ++j) {
         const float2 d = convex_hull[j] - origin;
         const float u0_dot_d = dot(u0, d);

         if (u0_dot_d < min0) {
            min0 = u0_dot_d;
         }
         else if (u0_dot_d > max0) {
            max0 = u0_dot_d;
         }

         const float u1_dot_d = dot(u1, d);

         if (u1_dot_d < min1) {
            min1 = u1_dot_d;
         }
         else if (u1_dot_d > max1) {
            max1 = u1_dot_d;
         }
      }

      const float area = (max0 - min0) * (max1 - min1);

      if (area < min_area) {
         centre = origin + ((min0 + max0) / 2.0f) * u0 + ((max1 + min1) / 2.0f) * u1;
         axes[0] = u0;
         axes[1] = u1;
         extents = {(max0 - min0) / 2.0f, (max1 - min1) / 2.0f};
         min_area = area;
      }
   }

   const float y =
      std::min_element(points.begin(), points.end(), [](const float3& l, const float3& r) {
         return l.y < r.y;
      })->y;
   const float angle = std::atan2(axes[1].x, axes[0].x);

   return {.position = {centre.x, y, centre.y}, .size = extents, .rotation_angle = angle};
}

auto barrier_from_object(const object& object,
                         const object_class_library& object_classes) noexcept -> barrier_metrics
{
   std::array<float3, 8> corners =
      to_corners(object_classes[object.class_handle].model->collision_bounding_box);

   for (auto& v : corners) v = object.rotation * v + object.position;

   return barrier_from_points(corners);
}

}