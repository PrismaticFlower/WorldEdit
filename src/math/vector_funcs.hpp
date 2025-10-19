#pragma once

#include "scalar_funcs.hpp"
#include "types.hpp"

#include <cassert>
#include <cmath>
#include <utility>

namespace we {

constexpr auto operator+(const float2& v) noexcept -> float2
{
   return {v.x, v.y};
}

constexpr auto operator-(const float2& v) noexcept -> float2
{
   return {-v.x, -v.y};
}

constexpr auto operator-(const float3& v) noexcept -> float3
{
   return {-v.x, -v.y, -v.z};
}

constexpr auto operator-(const float4& v) noexcept -> float4
{
   return {-v.x, -v.y, -v.z, -v.w};
}

constexpr auto operator+(const float2& a, const float2& b) noexcept -> float2
{
   return {a.x + b.x, a.y + b.y};
}

constexpr auto operator+(const float3& a, const float3& b) noexcept -> float3
{
   return {a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr auto operator+(const float4& a, const float4& b) noexcept -> float4
{
   return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr auto operator-(const float2& a, const float2& b) noexcept -> float2
{
   return {a.x - b.x, a.y - b.y};
}

constexpr auto operator-(const float3& a, const float3& b) noexcept -> float3
{
   return {a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr auto operator-(const float4& a, const float4& b) noexcept -> float4
{
   return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr auto operator*(const float2& a, const float2& b) noexcept -> float2
{
   return {a.x * b.x, a.y * b.y};
}

constexpr auto operator*(const float3& a, const float3& b) noexcept -> float3
{
   return {a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr auto operator*(const float4& a, const float4& b) noexcept -> float4
{
   return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr auto operator/(const float2& a, const float2& b) noexcept -> float2
{
   return {a.x / b.x, a.y / b.y};
}

constexpr auto operator/(const float3& a, const float3& b) noexcept -> float3
{
   return {a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr auto operator/(const float4& a, const float4& b) noexcept -> float4
{
   return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr auto operator+=(float2& a, const float2& b) noexcept -> float2&
{
   a.x += b.x;
   a.y += b.y;

   return a;
}

constexpr auto operator+=(float3& a, const float3& b) noexcept -> float3&
{
   a.x += b.x;
   a.y += b.y;
   a.z += b.z;

   return a;
}

constexpr auto operator+=(float4& a, const float4& b) noexcept -> float4&
{
   a.x += b.x;
   a.y += b.y;
   a.z += b.z;
   a.w += b.w;

   return a;
}

constexpr auto operator-=(float2& a, const float2& b) noexcept -> float2&
{
   a.x -= b.x;
   a.y -= b.y;

   return a;
}

constexpr auto operator-=(float3& a, const float3& b) noexcept -> float3&
{
   a.x -= b.x;
   a.y -= b.y;
   a.z -= b.z;

   return a;
}

constexpr auto operator-=(float4& a, const float4& b) noexcept -> float4&
{
   a.x -= b.x;
   a.y -= b.y;
   a.z -= b.z;
   a.w -= b.w;

   return a;
}

constexpr auto operator*=(float2& a, const float2& b) noexcept -> float2&
{
   a.x *= b.x;
   a.y *= b.y;

   return a;
}

constexpr auto operator*=(float3& a, const float3& b) noexcept -> float3&
{
   a.x *= b.x;
   a.y *= b.y;
   a.z *= b.z;

   return a;
}

constexpr auto operator*=(float4& a, const float4& b) noexcept -> float4&
{
   a.x *= b.x;
   a.y *= b.y;
   a.z *= b.z;
   a.w *= b.w;

   return a;
}

constexpr auto operator/=(float2& a, const float2& b) noexcept -> float2&
{
   a.x /= b.x;
   a.y /= b.y;

   return a;
}

constexpr auto operator/=(float3& a, const float3& b) noexcept -> float3&
{
   a.x /= b.x;
   a.y /= b.y;
   a.z /= b.z;

   return a;
}

constexpr auto operator/=(float4& a, const float4& b) noexcept -> float4&
{
   a.x /= b.x;
   a.y /= b.y;
   a.z /= b.z;
   a.w /= b.w;

   return a;
}

constexpr auto operator+(const float2& a, const float b) noexcept -> float2
{
   return {a.x + b, a.y + b};
}

constexpr auto operator+(const float3& a, const float b) noexcept -> float3
{
   return {a.x + b, a.y + b, a.z + b};
}

constexpr auto operator+(const float4& a, const float b) noexcept -> float4
{
   return {a.x + b, a.y + b, a.z + b, a.w + b};
}

constexpr auto operator-(const float2& a, const float b) noexcept -> float2
{
   return {a.x - b, a.y - b};
}

constexpr auto operator-(const float3& a, const float b) noexcept -> float3
{
   return {a.x - b, a.y - b, a.z - b};
}

constexpr auto operator-(const float4& a, const float b) noexcept -> float4
{
   return {a.x - b, a.y - b, a.z - b, a.w - b};
}

constexpr auto operator*(const float2& a, const float b) noexcept -> float2
{
   return {a.x * b, a.y * b};
}

constexpr auto operator*(const float3& a, const float b) noexcept -> float3
{
   return {a.x * b, a.y * b, a.z * b};
}

constexpr auto operator*(const float4& a, const float b) noexcept -> float4
{
   return {a.x * b, a.y * b, a.z * b, a.w * b};
}

constexpr auto operator/(const float2& a, const float b) noexcept -> float2
{
   return {a.x / b, a.y / b};
}

constexpr auto operator/(const float3& a, const float b) noexcept -> float3
{
   return {a.x / b, a.y / b, a.z / b};
}

constexpr auto operator/(const float4& a, const float b) noexcept -> float4
{
   return {a.x / b, a.y / b, a.z / b, a.w / b};
}

constexpr auto operator+=(float2& a, const float b) noexcept -> float2
{
   a.x += b;
   a.y += b;

   return a;
}

constexpr auto operator+=(float3& a, const float b) noexcept -> float3&
{
   a.x += b;
   a.y += b;
   a.z += b;

   return a;
}

constexpr auto operator+=(float4& a, const float b) noexcept -> float4&
{
   a.x += b;
   a.y += b;
   a.z += b;
   a.w += b;

   return a;
}

constexpr auto operator-=(float2& a, const float b) noexcept -> float2
{
   a.x -= b;
   a.y -= b;

   return a;
}

constexpr auto operator-=(float3& a, const float b) noexcept -> float3&
{
   a.x -= b;
   a.y -= b;
   a.z -= b;

   return a;
}

constexpr auto operator-=(float4& a, const float b) noexcept -> float4&
{
   a.x -= b;
   a.y -= b;
   a.z -= b;
   a.w -= b;

   return a;
}

constexpr auto operator*=(float2& a, const float b) noexcept -> float2
{
   a.x *= b;
   a.y *= b;

   return a;
}

constexpr auto operator*=(float3& a, const float b) noexcept -> float3&
{
   a.x *= b;
   a.y *= b;
   a.z *= b;

   return a;
}

constexpr auto operator*=(float4& a, const float b) noexcept -> float4&
{
   a.x *= b;
   a.y *= b;
   a.z *= b;
   a.w *= b;

   return a;
}

constexpr auto operator/=(float2& a, const float b) noexcept -> float2
{
   a.x /= b;
   a.y /= b;

   return a;
}

constexpr auto operator/=(float3& a, const float b) noexcept -> float3&
{
   a.x /= b;
   a.y /= b;
   a.z /= b;

   return a;
}

constexpr auto operator/=(float4& a, const float b) noexcept -> float4&
{
   a.x /= b;
   a.y /= b;
   a.z /= b;
   a.w /= b;

   return a;
}

constexpr auto operator+(const float a, const float2& b) noexcept -> float2
{
   return {a + b.x, a + b.y};
}

constexpr auto operator+(const float a, const float3& b) noexcept -> float3
{
   return {a + b.x, a + b.y, a + b.z};
}

constexpr auto operator+(const float a, const float4& b) noexcept -> float4
{
   return {a + b.x, a + b.y, a + b.z, a + b.w};
}

constexpr auto operator-(const float a, const float2& b) noexcept -> float2
{
   return {a - b.x, a - b.y};
}

constexpr auto operator-(const float a, const float3& b) noexcept -> float3
{
   return {a - b.x, a - b.y, a - b.z};
}

constexpr auto operator-(const float a, const float4& b) noexcept -> float4
{
   return {a - b.x, a - b.y, a - b.z, a - b.w};
}

constexpr auto operator*(const float a, const float2& b) noexcept -> float2
{
   return {a * b.x, a * b.y};
}

constexpr auto operator*(const float a, const float3& b) noexcept -> float3
{
   return {a * b.x, a * b.y, a * b.z};
}

constexpr auto operator*(const float a, const float4& b) noexcept -> float4
{
   return {a * b.x, a * b.y, a * b.z, a * b.w};
}

constexpr auto operator/(const float a, const float2& b) noexcept -> float2
{
   return {a / b.x, a / b.y};
}

constexpr auto operator/(const float a, const float3& b) noexcept -> float3
{
   return {a / b.x, a / b.y, a / b.z};
}

constexpr auto operator/(const float a, const float4& b) noexcept -> float4
{
   return {a / b.x, a / b.y, a / b.z, a / b.w};
}

constexpr auto abs(const float2& v) -> float2
{
   return {v.x < 0.0f ? -v.x : v.x, v.y < 0.0f ? -v.y : v.y};
}

constexpr auto abs(const float3& v) -> float3
{
   return {v.x < 0.0f ? -v.x : v.x, v.y < 0.0f ? -v.y : v.y, v.z < 0.0f ? -v.z : v.z};
}

constexpr auto abs(const float4& v) -> float4
{
   return {v.x < 0.0f ? -v.x : v.x, v.y < 0.0f ? -v.y : v.y,
           v.z < 0.0f ? -v.z : v.z, v.w < 0.0f ? -v.w : v.w};
}

inline auto ceil(const float2& v) -> float2
{
   return {std::ceil(v.x), std::ceil(v.y)};
}

inline auto ceil(const float3& v) -> float3
{
   return {std::ceil(v.x), std::ceil(v.y), std::ceil(v.z)};
}

inline auto ceil(const float4& v) -> float4
{
   return {std::ceil(v.x), std::ceil(v.y), std::ceil(v.z), std::ceil(v.w)};
}

inline auto floor(const float2& v) -> float2
{
   return {std::floor(v.x), std::floor(v.y)};
}

inline auto floor(const float3& v) -> float3
{
   return {std::floor(v.x), std::floor(v.y), std::floor(v.z)};
}

inline auto floor(const float4& v) -> float4
{
   return {std::floor(v.x), std::floor(v.y), std::floor(v.z), std::floor(v.w)};
}

inline auto round(const float2& v) noexcept -> float2
{
   return {std::round(v.x), std::round(v.y)};
}

inline auto round(const float3& v) noexcept -> float3
{
   return {std::round(v.x), std::round(v.y), std::round(v.z)};
}

inline auto round(const float4& v) noexcept -> float4
{
   return {std::round(v.x), std::round(v.y), std::round(v.z), std::round(v.w)};
}

inline auto trunc(const float2& v) noexcept -> float2
{
   return {std::trunc(v.x), std::trunc(v.y)};
}

inline auto trunc(const float3& v) noexcept -> float3
{
   return {std::trunc(v.x), std::trunc(v.y), std::trunc(v.z)};
}

inline auto trunc(const float4& v) noexcept -> float4
{
   return {std::trunc(v.x), std::trunc(v.y), std::trunc(v.z), std::trunc(v.w)};
}

inline auto frac(const float2& v) -> float2
{
   return v - floor(v);
}

inline auto frac(const float3& v) -> float3
{
   return v - floor(v);
}

inline auto frac(const float4& v) -> float4
{
   return v - floor(v);
}

constexpr auto min(const float2& a, const float2& b) noexcept -> float2
{
   return {a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y};
}

constexpr auto min(const float3& a, const float3& b) noexcept -> float3
{
   return {a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z};
}

constexpr auto min(const float4& a, const float4& b) noexcept -> float4
{
   return {a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z,
           a.w < b.w ? a.w : b.w};
}

constexpr auto max(const float2& a, const float2& b) noexcept -> float2
{
   return {a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y};
}

constexpr auto max(const float3& a, const float3& b) noexcept -> float3
{
   return {a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z};
}

constexpr auto max(const float4& a, const float4& b) noexcept -> float4
{
   return {a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z,
           a.w > b.w ? a.w : b.w};
}

constexpr auto clamp(const float2& v, const float2& min_v, const float2& max_v) noexcept
   -> float2
{
   return max(min(v, max_v), min_v);
}

constexpr auto clamp(const float3& v, const float3& min_v, const float3& max_v) noexcept
   -> float3
{
   return max(min(v, max_v), min_v);
}

constexpr auto clamp(const float4& v, const float4& min_v, const float4& max_v) noexcept
   -> float4
{
   return max(min(v, max_v), min_v);
}

constexpr auto clamp(const float2& v, const float min_v, const float max_v) noexcept
   -> float2
{
   return max(min(v, float2{max_v, max_v}), float2{min_v, min_v});
}

constexpr auto clamp(const float3& v, const float min_v, const float max_v) noexcept
   -> float3
{
   return max(min(v, float3{max_v, max_v, max_v}), float3{min_v, min_v, min_v});
}

constexpr auto clamp(const float4& v, const float min_v, const float max_v) noexcept
   -> float4
{
   return max(min(v, float4{max_v, max_v, max_v, max_v}),
              float4{min_v, min_v, min_v, min_v});
}

constexpr auto saturate(const float2& v) noexcept -> float2
{
   return clamp(v, 0.0f, 1.0f);
}

constexpr auto saturate(const float3& v) noexcept -> float3
{
   return clamp(v, 0.0f, 1.0f);
}

constexpr auto saturate(const float4& v) noexcept -> float4
{
   return clamp(v, 0.0f, 1.0f);
}

constexpr auto sign(const float2& v) noexcept -> float2
{
   return {v.x < 0.0f ? -1.0f : 1.0f, v.y < 0.0f ? -1.0f : 1.0f};
}

constexpr auto sign(const float3& v) noexcept -> float3
{
   return {v.x < 0.0f ? -1.0f : 1.0f, v.y < 0.0f ? -1.0f : 1.0f,
           v.z < 0.0f ? -1.0f : 1.0f};
}

constexpr auto sign(const float4& v) noexcept -> float4
{
   return {v.x < 0.0f ? -1.0f : 1.0f, v.y < 0.0f ? -1.0f : 1.0f,
           v.z < 0.0f ? -1.0f : 1.0f, v.w < 0.0f ? -1.0f : 1.0f};
}

constexpr auto step(const float2& a, const float2& b) noexcept -> float2
{
   return {a.x < b.x ? 1.0f : 0.0f, a.y < b.y ? 1.0f : 0.0f};
}

constexpr auto step(const float3& a, const float3& b) noexcept -> float3
{
   return {a.x < b.x ? 1.0f : 0.0f, a.y < b.y ? 1.0f : 0.0f, a.z < b.z ? 1.0f : 0.0f};
}

constexpr auto step(const float4& a, const float4& b) noexcept -> float4
{
   return {a.x < b.x ? 1.0f : 0.0f, a.y < b.y ? 1.0f : 0.0f,
           a.z < b.z ? 1.0f : 0.0f, a.w < b.w ? 1.0f : 0.0f};
}

constexpr auto dot(const float2& a, const float2& b) noexcept -> float
{
   return a.x * b.x + a.y * b.y;
}

constexpr auto dot(const float3& a, const float3& b) noexcept -> float
{
   return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr auto dot(const float4& a, const float4& b) noexcept -> float
{
   return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline auto length(const float2& v) noexcept -> float
{
   return sqrt(dot(v, v));
}

inline auto length(const float3& v) noexcept -> float
{
   return sqrt(dot(v, v));
}

inline auto length(const float4& v) noexcept -> float
{
   return sqrt(dot(v, v));
}

inline auto distance(const float2& a, const float2& b) noexcept -> float
{
   return length(b - a);
}

inline auto distance(const float3& a, const float3& b) noexcept -> float
{
   return length(b - a);
}

inline auto distance(const float4& a, const float4& b) noexcept -> float
{
   return length(b - a);
}

inline auto normalize(const float2& v) noexcept -> float2
{
   return v / length(v);
}

inline auto normalize(const float3& v) noexcept -> float3
{
   return v / length(v);
}

inline auto normalize(const float4& v) noexcept -> float4
{
   return v / length(v);
}

inline auto lerp(const float2& a, const float2& b, const float s) noexcept -> float2
{
   return (1.0f - s) * a + s * b;
}

inline auto lerp(const float3& a, const float3& b, const float s) noexcept -> float3
{
   return (1.0f - s) * a + s * b;
}

inline auto lerp(const float4& a, const float4& b, const float s) noexcept -> float4
{
   return (1.0f - s) * a + s * b;
}

constexpr auto cross(const float3& a, const float3& b) noexcept -> float3
{
   return {a.y * b.z - b.y * a.z, //
           a.z * b.x - b.z * a.x, //
           a.x * b.y - b.x * a.y};
}

inline auto cos(const float2& v) noexcept -> float2
{
   return {std::cos(v.x), std::cos(v.y)};
}

inline auto cos(const float3& v) noexcept -> float3
{
   return {std::cos(v.x), std::cos(v.y), std::cos(v.z)};
}

inline auto cos(const float4& v) noexcept -> float4
{
   return {std::cos(v.x), std::cos(v.y), std::cos(v.z), std::cos(v.w)};
}

inline auto sin(const float2& v) noexcept -> float2
{
   return {std::sin(v.x), std::sin(v.y)};
}

inline auto sin(const float3& v) noexcept -> float3
{
   return {std::sin(v.x), std::sin(v.y), std::sin(v.z)};
}

inline auto sin(const float4& v) noexcept -> float4
{
   return {std::sin(v.x), std::sin(v.y), std::sin(v.z), std::sin(v.w)};
}

inline auto sqrt(const float2& v) noexcept -> float2
{
   return {sqrt(v.x), sqrt(v.y)};
}

inline auto sqrt(const float3& v) noexcept -> float3
{
   return {sqrt(v.x), sqrt(v.y), sqrt(v.z)};
}

inline auto sqrt(const float4& v) noexcept -> float4
{
   return {sqrt(v.x), sqrt(v.y), sqrt(v.z), sqrt(v.w)};
}

}
