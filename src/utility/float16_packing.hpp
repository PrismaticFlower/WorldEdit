#pragma once

#include "types.hpp"

#include <array>

namespace we::utility {

auto pack_float16(const float v) noexcept -> uint16;

auto pack_float16(const float2& v) noexcept -> std::array<uint16, 2>;

auto pack_float16(const float3& v) noexcept -> std::array<uint16, 3>;

auto pack_float16(const float4& v) noexcept -> std::array<uint16, 4>;

auto unpack_float16(const uint16 v) noexcept -> float;

auto unpack_float16(const std::array<uint16, 2>& v) noexcept -> float2;

auto unpack_float16(const std::array<uint16, 3>& v) noexcept -> float3;

auto unpack_float16(const std::array<uint16, 4>& v) noexcept -> float4;

}