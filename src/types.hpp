#pragma once

#include <concepts>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace we {

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;

using double2 = glm::dvec2;
using double3 = glm::dvec3;
using double4 = glm::dvec4;

using quaternion = glm::quat;
using float4x4 = glm::mat4;
using double4x4 = glm::dmat4;
using float3x3 = glm::mat3;
using double3x3 = glm::dmat4;

static_assert(sizeof(float2) == 8);
static_assert(sizeof(float3) == 12);
static_assert(sizeof(float4) == 16);

static_assert(sizeof(double2) == 16);
static_assert(sizeof(double3) == 24);
static_assert(sizeof(double4) == 32);

static_assert(sizeof(quaternion) == 16);
static_assert(sizeof(float4x4) == 64);
static_assert(sizeof(double4x4) == 128);

constexpr auto to_int8(const std::convertible_to<int8> auto& value) noexcept -> int8
{
   return static_cast<int8>(value);
}

constexpr auto to_int16(const std::convertible_to<int16> auto& value) noexcept -> int16
{
   return static_cast<int16>(value);
}

constexpr auto to_int32(const std::convertible_to<int32> auto& value) noexcept -> int32
{
   return static_cast<int32>(value);
}

constexpr auto to_int64(const std::convertible_to<int64> auto& value) noexcept -> int64
{
   return static_cast<int64>(value);
}

constexpr auto to_uint8(const std::convertible_to<uint8> auto& value) noexcept -> uint8
{
   return static_cast<uint8>(value);
}

constexpr auto to_uint16(const std::convertible_to<uint16> auto& value) noexcept -> uint16
{
   return static_cast<uint16>(value);
}

constexpr auto to_uint32(const std::convertible_to<uint32> auto& value) noexcept -> uint32
{
   return static_cast<uint32>(value);
}

constexpr auto to_uint64(const std::convertible_to<uint64> auto& value) noexcept -> uint64
{
   return static_cast<uint64>(value);
}

constexpr auto to_float(const std::convertible_to<float> auto& value) noexcept -> float
{
   return static_cast<float>(value);
}

constexpr auto to_double(const std::convertible_to<double> auto& value) noexcept -> double
{
   return static_cast<double>(value);
}

}
