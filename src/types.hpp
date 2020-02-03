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

using glm::vec1;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::dvec1;
using glm::dvec2;
using glm::dvec3;
using glm::dvec4;

using glm::uvec1;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;

using glm::ivec1;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;

using glm::u8vec1;
using glm::u8vec2;
using glm::u8vec3;
using glm::u8vec4;

using glm::i8vec1;
using glm::i8vec2;
using glm::i8vec3;
using glm::i8vec4;

using glm::u16vec1;
using glm::u16vec2;
using glm::u16vec3;
using glm::u16vec4;

using glm::i16vec1;
using glm::i16vec2;
using glm::i16vec3;
using glm::i16vec4;

using glm::u32vec1;
using glm::u32vec2;
using glm::u32vec3;
using glm::u32vec4;

using glm::i32vec1;
using glm::i32vec2;
using glm::i32vec3;
using glm::i32vec4;

using glm::u64vec1;
using glm::u64vec2;
using glm::u64vec3;
using glm::u64vec4;

using glm::i64vec1;
using glm::i64vec2;
using glm::i64vec3;
using glm::i64vec4;

using quaternion = glm::quat;
using matrix4x4 = glm::mat4;

}