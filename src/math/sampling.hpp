#pragma once

#include "vector_funcs.hpp"

#include <numbers>

namespace we {

inline auto concentric_sample_disk(const float2 u) noexcept -> float2
{
   constexpr float pi_over_2 = std::numbers::pi_v<float> / 2.0f;
   constexpr float pi_over_4 = std::numbers::pi_v<float> / 4.0f;

   const float2 offset = 2.0f * u - 1.0f;

   if (offset.x == 0.0f and offset.y == 0.0f) return {};

   float theta = 0.0f;
   float r = 0.0f;

   if (std::abs(offset.x) > std::abs(offset.y)) {
      r = offset.x;
      theta = pi_over_4 * (offset.y / offset.x);
   }
   else {
      r = offset.y;
      theta = pi_over_2 - pi_over_4 * (offset.x / offset.y);
   }

   return r * float2{std::cos(theta), std::sin(theta)};
}

inline auto cosine_sample_hemisphere(const float2 u) noexcept -> float3
{
   const float2 d = concentric_sample_disk(u);
   const float z = sqrt(std::max(0.0f, 1.0f - d.x * d.x - d.y * d.y));

   return float3{d.x, d.y, z};
}

inline auto uniform_sample_hemisphere(const float u, const float v) noexcept -> float3
{
   const float z = u;
   const float r = sqrt(std::max(0.0f, 1.0f - z * z));
   const float phi = 2.0f * std::numbers::pi_v<float> * v;

   return {r * std::cos(phi), r * std::sin(phi), z};
}

inline auto uniform_sample_triangle(const float2 u) noexcept -> float3
{
   float w0;
   float w1;

   if (u.x + u.y < 1.0f) {
      w0 = u.x;
      w1 = u.y;
   }
   else {
      w0 = 1.0f - u.x;
      w1 = 1.0f - u.y;
   }

   return float3{w0, w1, 1.0f - w0 - w1};
}

// Martin Roberts - The Unreasonable Effectiveness of Quasirandom Sequences https://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
inline auto R2(int32 i) noexcept -> float2
{
   constexpr float g = 1.32471795724474602596f;
   constexpr float a1 = 1.0f / g;
   constexpr float a2 = 1.0f / (g * g);

   return frac(float2{a1 * i, a2 * i});
}

}
