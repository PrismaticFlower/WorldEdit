#pragma once

#include "types.hpp"

namespace we::math {

struct bounding_box {
   float3 min{};
   float3 max{};
};

auto combine(const bounding_box& l, const bounding_box& r) noexcept -> bounding_box;

auto integrate(const bounding_box& box, const float3& v) noexcept -> bounding_box;

auto operator*(const quaternion& quat, const bounding_box& box) noexcept -> bounding_box;

auto operator+(const bounding_box& box, const float3& offset) noexcept -> bounding_box;

}
