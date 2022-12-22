#pragma once

#include "types.hpp"
#include "vector_funcs.hpp"

namespace we {

constexpr auto operator*(const float4x4& matrix, const float4& vec) noexcept -> float4
{
   return (matrix[0] * vec.x) + (matrix[1] * vec.y) + (matrix[2] * vec.z) +
          (matrix[3] * vec.w);
}

constexpr auto operator*(const float4x4& matrix, const float3& vec) noexcept -> float3
{
   const float4 transformed_vec = matrix * float4{vec, 1.0f};

   return {transformed_vec.x, transformed_vec.y, transformed_vec.z};
}

constexpr auto operator*(const float4x4& a, const float4x4& b) noexcept -> float4x4
{
   return {{a[0] * b[0].x + a[1] * b[0].y + a[2] * b[0].z + a[3] * b[0].w},
           {a[0] * b[1].x + a[1] * b[1].y + a[2] * b[1].z + a[3] * b[1].w},
           {a[0] * b[2].x + a[1] * b[2].y + a[2] * b[2].z + a[3] * b[2].w},
           {a[0] * b[3].x + a[1] * b[3].y + a[2] * b[3].z + a[3] * b[3].w}};
}

constexpr auto transpose(const float4x4& matrix) -> float4x4
{
   return {{matrix[0].x, matrix[1].x, matrix[2].x, matrix[3].x},
           {matrix[0].y, matrix[1].y, matrix[2].y, matrix[3].y},
           {matrix[0].z, matrix[1].z, matrix[2].z, matrix[3].z},
           {matrix[0].w, matrix[1].w, matrix[2].w, matrix[3].w}};
}

auto inverse(const float4x4& matrix) -> float4x4;

inline auto look_at_lh(const float3& eye_position, const float3& focus_position,
                       const float3& up_direction) noexcept -> float4x4
{
   const float3 eye_direction = normalize(focus_position - eye_position);

   const float3 r0 = normalize(cross(up_direction, eye_direction));
   const float3 r1 = cross(eye_direction, r0);

   const float3 neg_eye_position = -eye_position;

   return transpose({{r0, dot(r0, neg_eye_position)},
                     {r1, dot(r1, neg_eye_position)},
                     {eye_direction, dot(eye_direction, neg_eye_position)},
                     {0.0f, 0.0f, 0.0f, 1.0f}});
}

inline auto make_rotation_matrix_from_euler(const float3& euler) noexcept -> float4x4
{
   const float3 cos = we::cos(euler);
   const float3 sin = we::sin(euler);

   return {{cos.y * cos.z + sin.y * sin.x * sin.z, cos.x * sin.z,
            sin.z * cos.y * sin.x - sin.y * cos.z, 0.0f}, //
           {cos.z * sin.y * sin.x - sin.z * cos.y, cos.z * cos.x,
            sin.y * sin.z + cos.z * cos.y * sin.x, 0.0f}, //
           {cos.x * sin.y, -sin.x, cos.x * cos.y, 0.0f},  //
           {0.0f, 0.0f, 0.0f, 1.0f}};
}

constexpr auto operator*(const float3x3& matrix, const float3& vec) noexcept -> float3
{
   return {matrix[0].x * vec.x + matrix[1].x * vec.y + matrix[2].x * vec.z,
           matrix[0].y * vec.x + matrix[1].y * vec.y + matrix[2].y * vec.z,
           matrix[0].z * vec.x + matrix[1].z * vec.y + matrix[2].z * vec.z};
}

}