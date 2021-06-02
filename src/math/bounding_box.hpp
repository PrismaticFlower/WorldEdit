#pragma once

#include "types.hpp"

#include <array>

namespace we::math {

struct bounding_box {
   float3 min{};
   float3 max{};
};

inline auto combine(const bounding_box& l, const bounding_box& r) noexcept -> bounding_box
{
   return {.min = glm::min(l.min, r.min), .max = glm::max(l.max, r.max)};
}

inline auto integrate(const bounding_box& box, const float3& v) noexcept -> bounding_box
{
   return {.min = glm::min(box.min, v), .max = glm::max(box.max, v)};
}

inline auto operator*(const quaternion& quat, const bounding_box& box) noexcept -> bounding_box
{
   const float3 centre = (box.max + box.min) / 2.0f;
   const float3 size = (box.max - box.min) / 2.0f;

   const std::array<float3, 8>
      vertices{// top corners
               quat * (centre + float3{size.x, size.y, size.z}),
               quat * (centre + float3{-size.x, size.y, size.z}),
               quat * (centre + float3{-size.x, size.y, -size.z}),
               quat * (centre + float3{size.x, size.y, -size.z}),
               // bottom corners
               quat * (centre + float3{size.x, -size.y, size.z}),
               quat * (centre + float3{-size.x, -size.y, size.z}),
               quat * (centre + float3{-size.x, -size.y, -size.z}),
               quat * (centre + float3{size.x, -size.y, -size.z})};

   bounding_box new_box{vertices[0], vertices[0]};

   for (std::size_t i = 1; i < vertices.size(); ++i) {
      new_box.min = glm::min(new_box.min, vertices[i]);
      new_box.max = glm::max(new_box.max, vertices[i]);
   }

   return new_box;
}

inline auto operator+(const bounding_box& box, const float3& offset) noexcept -> bounding_box
{
   return {.min = box.min + offset, .max = box.max + offset};
}
}
