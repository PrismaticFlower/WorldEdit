#pragma once

#include "types.hpp"

#include <array>

namespace we::math {

struct bounding_box {
   float3 min{};
   float3 max{};
};

auto combine(const bounding_box& l, const bounding_box& r) noexcept -> bounding_box;

auto integrate(const bounding_box& box, const float3& v) noexcept -> bounding_box;

auto to_corners(const bounding_box& box) noexcept -> std::array<float3, 8>;

auto operator*(const quaternion& quat, const bounding_box& box) noexcept -> bounding_box;

auto operator+(const bounding_box& box, const float3& offset) noexcept -> bounding_box;

}
