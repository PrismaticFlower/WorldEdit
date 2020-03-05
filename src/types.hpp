#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sk {

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

}