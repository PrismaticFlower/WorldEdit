#pragma once

#include "catch.hpp"
#include "types.hpp"

#include <cmath>
#include <type_traits>

namespace we {

inline bool approx_equals(const float2& l, const float2& r)
{
   return l.x == Approx(r.x) and l.y == Approx(r.y);
}

inline bool approx_equals(const float3& l, const float3& r)
{
   return l.x == Approx(r.x) and l.y == Approx(r.y) and l.z == Approx(r.z);
}

inline bool approx_equals(const float4& l, const float4& r)
{
   return l.x == Approx(r.x) and l.y == Approx(r.y) and l.z == Approx(r.z) and
          l.w == Approx(r.w);
}

inline bool approx_equals(const quaternion& l, const quaternion& r)
{
   return l.x == Approx(r.x) and l.y == Approx(r.y) and l.z == Approx(r.z) and
          l.w == Approx(r.w);
}

inline bool approx_equals(const float4& l, const float4& r, const float epsilon)
{
   using std::abs;

   return abs(l.x - r.x) < epsilon and abs(l.y - r.y) < epsilon and
          abs(l.z - r.z) < epsilon and abs(l.w - r.w) < epsilon;
}

}
