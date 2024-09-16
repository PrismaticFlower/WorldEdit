
#include "bounding_box.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <array>

namespace we::math {

auto combine(const bounding_box& l, const bounding_box& r) noexcept -> bounding_box
{
   return {.min = min(l.min, r.min), .max = max(l.max, r.max)};
}

auto integrate(const bounding_box& box, const float3& v) noexcept -> bounding_box
{
   return {.min = min(box.min, v), .max = max(box.max, v)};
}

auto to_corners(const bounding_box& box) noexcept -> std::array<float3, 8>
{
   const float3 centre = (box.max + box.min) / 2.0f;
   const float3 size = (box.max - box.min) / 2.0f;

   // Don't change the ordering of the corners, other code is allowed to depend on it.

   return {// top corners
           (centre + float3{size.x, size.y, size.z}),
           (centre + float3{-size.x, size.y, size.z}),
           (centre + float3{-size.x, size.y, -size.z}),
           (centre + float3{size.x, size.y, -size.z}),
           // bottom corners
           (centre + float3{size.x, -size.y, size.z}),
           (centre + float3{-size.x, -size.y, size.z}),
           (centre + float3{-size.x, -size.y, -size.z}),
           (centre + float3{size.x, -size.y, -size.z})};
}

auto operator*(const quaternion& quat, const bounding_box& box) noexcept -> bounding_box
{
   std::array<float3, 8> vertices = {
      // top corners
      float3{box.max.x, box.max.y, box.max.z},
      float3{box.min.x, box.max.y, box.max.z},
      float3{box.min.x, box.max.y, box.min.z},
      float3{box.max.x, box.max.y, box.min.z},
      // bottom corners
      float3{box.max.x, box.min.y, box.max.z},
      float3{box.min.x, box.min.y, box.max.z},
      float3{box.min.x, box.min.y, box.min.z},
      float3{box.max.x, box.min.y, box.min.z},
   };

   for (float3& v : vertices) v = quat * v;

   bounding_box new_box{vertices[0], vertices[0]};

   for (std::size_t i = 1; i < vertices.size(); ++i) {
      new_box.min = min(new_box.min, vertices[i]);
      new_box.max = max(new_box.max, vertices[i]);
   }

   return new_box;
}

auto operator+(const bounding_box& box, const float3& offset) noexcept -> bounding_box
{
   return {.min = box.min + offset, .max = box.max + offset};
}

}