#include "bounding_box.hpp"
#include "vector_funcs.hpp"

#include <float.h> // FLT_EPSILON

#include <utility> // std::min, std::max (could replace)

namespace we {

inline bool intersect_tri(const float3& ray_origin, const float3& ray_direction,
                          const float3& v0, const float3& v1, const float3& v2,
                          float& distance) noexcept
{
   const float3 edge1 = v1 - v0;
   const float3 edge2 = v2 - v0;

   const float3 rd_cross_edge2 = cross(ray_direction, edge2);
   const float det = dot(edge1, rd_cross_edge2);

   const float epsilon = FLT_EPSILON;

   if (det > -epsilon and det < epsilon) return false;

   const float inv_det = 1.0f / det;

   const float3 s = ray_origin - v0;
   const float u = inv_det * dot(s, rd_cross_edge2);

   if (u < 0.0f or u > 1.0f) return false;

   const float3 s_cross_e1 = cross(s, edge1);
   const float v = inv_det * dot(ray_direction, s_cross_e1);

   if (v < 0.0f or u + v > 1.0f) return false;

   const float t = inv_det * dot(edge2, s_cross_e1);

   const bool hit = t > 0.0f;

   if (hit) {
      distance = t;

      return true;
   }

   return false;
}

inline bool intersect_aabb(const float3& ray_origin, const float3& inv_ray_direction,
                           const math::bounding_box& bbox, const float t_limit,
                           float& t) noexcept
{
   const float3 ts0 = (bbox.min - ray_origin) * inv_ray_direction;
   const float3 ts1 = (bbox.max - ray_origin) * inv_ray_direction;

   const float3 ts_min = min(ts0, ts1);
   const float3 ts_max = max(ts0, ts1);

   const float t_min = std::max(ts_min.x, std::max(ts_min.y, ts_min.z));
   const float t_max =
      std::min(std::min(ts_max.x, std::min(ts_max.y, ts_max.z)), t_limit);

   if (t_min <= t_max) {
      t = t_min;

      return true;
   }

   return false;
}

}