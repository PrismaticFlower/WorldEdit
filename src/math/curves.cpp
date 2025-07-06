#include "curves.hpp"

#include "math/vector_funcs.hpp"

namespace we {

auto cubic_bezier(const float2& p0, const float2& p1, const float2& p2,
                  const float2& p3, const float t) noexcept -> float2
{
   const float t2 = t * t;
   const float t3 = t2 * t;

   const float inv_t = 1.0f - t;
   const float inv_t2 = inv_t * inv_t;
   const float inv_t3 = inv_t2 * inv_t;

   const float2 a = inv_t3 * p0;
   const float2 b = 3.0f * inv_t2 * t * p1;
   const float2 c = 3.0f * inv_t * t2 * p2;
   const float2 d = t3 * p3;

   return a + b + c + d;
}

auto cubic_bezier_tangent(const float2& p0, const float2& p1, const float2& p2,
                          const float2& p3, const float t) noexcept -> float2
{
   const float t2 = t * t;
   const float inv_t = 1.0f - t;
   const float inv_t2 = inv_t * inv_t;

   return normalize(3.0f * inv_t2 * (p1 - p0) + 6.0f * inv_t * t * (p2 - p1) +
                    3.0f * t2 * (p3 - p2));
}

auto cubic_bezier(const float3& p0, const float3& p1, const float3& p2,
                  const float3& p3, const float t) noexcept -> float3
{
   const float t2 = t * t;
   const float t3 = t2 * t;

   const float inv_t = 1.0f - t;
   const float inv_t2 = inv_t * inv_t;
   const float inv_t3 = inv_t2 * inv_t;

   const float3 a = inv_t3 * p0;
   const float3 b = 3.0f * inv_t2 * t * p1;
   const float3 c = 3.0f * inv_t * t2 * p2;
   const float3 d = t3 * p3;

   return a + b + c + d;
}

auto cubic_bezier_tangent(const float3& p0, const float3& p1, const float3& p2,
                          const float3& p3, const float t) noexcept -> float3
{
   const float t2 = t * t;
   const float inv_t = 1.0f - t;
   const float inv_t2 = inv_t * inv_t;

   return normalize(3.0f * inv_t2 * (p1 - p0) + 6.0f * inv_t * t * (p2 - p1) +
                    3.0f * t2 * (p3 - p2));
}

}