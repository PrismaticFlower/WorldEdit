#pragma once

#include "vector_funcs.hpp"

namespace we {

inline auto make_plane_from_point(float3 position, float3 normal) noexcept -> float4
{
   const float w = -dot(position, normal);

   return float4{normal, w};
}

inline auto make_plane(float3 v0, float3 v1, float3 v2) noexcept -> float4
{
   const float3 edge0 = v1 - v0;
   const float3 edge1 = v2 - v0;
   const float3 normal = normalize(cross(edge0, edge1));

   return float4{normal, -dot(normal, v0)};
}

inline auto intersect_plane(float3 ray_origin, float3 ray_direction,
                            float4 plane) noexcept -> float
{
   return -(dot(ray_origin, float3{plane.x, plane.y, plane.z}) + plane.w) /
          dot(ray_direction, float3{plane.x, plane.y, plane.z});
}

}