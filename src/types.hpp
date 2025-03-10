#pragma once

#include <cstdint>

namespace we {

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

struct float2 {
   float x = 0.0f;
   float y = 0.0f;

   constexpr float2() = default;
   constexpr float2(float x, float y) : x{x}, y{y} {};

   friend constexpr auto operator==(const float2& l, const float2& r) noexcept
      -> bool = default;
};

struct float3 {
   float x = 0.0f;
   float y = 0.0f;
   float z = 0.0f;

   constexpr float3() = default;
   constexpr float3(float x, float y, float z) : x{x}, y{y}, z{z} {};

   friend constexpr auto operator==(const float3& l, const float3& r) noexcept
      -> bool = default;
};

struct float4 {
   float x = 0.0f;
   float y = 0.0f;
   float z = 0.0f;
   float w = 0.0f;

   constexpr float4() = default;
   constexpr float4(float3 xyz, float w)
      : x{xyz.x}, y{xyz.y}, z{xyz.z}, w{w} {};
   constexpr float4(float x, float y, float z, float w)
      : x{x}, y{y}, z{z}, w{w} {};

   friend constexpr auto operator==(const float4& l, const float4& r) noexcept
      -> bool = default;
};

struct float4x4 {
   float4 rows[4] = {{1.0f, 0.0f, 0.0f, 0.0f},
                     {0.0f, 1.0f, 0.0f, 0.0f},
                     {0.0f, 0.0f, 1.0f, 0.0f},
                     {0.0f, 0.0f, 0.0f, 1.0f}};

   constexpr float4x4() = default;
   constexpr float4x4(float4 m0, float4 m1, float4 m2, float4 m3)
      : rows{m0, m1, m2, m3}
   {
   }

   constexpr auto operator[](this auto&& self, const uint64 i) noexcept -> auto&
   {
      return self.rows[i];
   }

   friend constexpr auto operator==(const float4x4& l, const float4x4& r) noexcept
      -> bool = default;
};

struct float3x3 {
   float3 rows[3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

   constexpr float3x3() = default;
   constexpr float3x3(float3 m0, float3 m1, float3 m2) : rows{m0, m1, m2} {}
   explicit constexpr float3x3(const float4x4& mat)
      : float3x3{{mat[0].x, mat[0].y, mat[0].z},
                 {mat[1].x, mat[1].y, mat[1].z},
                 {mat[2].x, mat[2].y, mat[2].z}}
   {
   }

   constexpr auto operator[](this auto&& self, const uint64 i) noexcept -> auto&
   {
      return self.rows[i];
   }

   friend constexpr auto operator==(const float3x3& l, const float3x3& r) noexcept
      -> bool = default;
};

struct quaternion {
   float w = 1.0f;
   float x = 0.0f;
   float y = 0.0f;
   float z = 0.0f;

   friend constexpr auto operator==(const quaternion& l, const quaternion& r) noexcept
      -> bool = default;
};

static_assert(sizeof(float2) == 8);
static_assert(sizeof(float3) == 12);
static_assert(sizeof(float4) == 16);

static_assert(sizeof(quaternion) == 16);
static_assert(sizeof(float4x4) == 64);
static_assert(sizeof(float3x3) == 36);

}
