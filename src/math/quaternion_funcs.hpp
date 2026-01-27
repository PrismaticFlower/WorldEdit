#pragma once

#include "matrix_funcs.hpp"
#include "scalar_funcs.hpp"
#include "types.hpp"
#include "vector_funcs.hpp"

#include <cmath>

namespace we {

constexpr auto operator*(const quaternion& quat, const float3& vec) noexcept -> float3
{
   const float3 u{quat.x, quat.y, quat.z};
   const float s = quat.w;

   return 2.0f * dot(u, vec) * u + (s * s - dot(u, u)) * vec +
          (2.0f * s * cross(u, vec));
}

constexpr auto operator*(const quaternion& a, const quaternion& b) noexcept -> quaternion
{
   return {a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
           a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
           a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
           a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x};
}

constexpr auto conjugate(const quaternion& quat) noexcept -> quaternion
{
   return {quat.w, -quat.x, -quat.y, -quat.z};
}

inline auto normalize(const quaternion& quat) noexcept -> quaternion
{
   float4 normalized = normalize(float4{quat.x, quat.y, quat.z, quat.w});

   return {normalized.w, normalized.x, normalized.y, normalized.z};
}

inline auto make_quat_from_matrix(const float3x3& matrix) noexcept -> quaternion
{
   quaternion quat;

   const float tr = matrix[0].x + matrix[1].y + matrix[2].z;

   if (tr > 0.0f) {
      const float s = sqrt(tr + 1.0f) * 2.0f;

      quat.w = 0.25f * s;
      quat.x = (matrix[1].z - matrix[2].y) / s;
      quat.y = (matrix[2].x - matrix[0].z) / s;
      quat.z = (matrix[0].y - matrix[1].x) / s;
   }
   else if ((matrix[0].x > matrix[1].y) and (matrix[0].x > matrix[2].z)) {
      const float s = sqrt(1.0f + matrix[0].x - matrix[1].y - matrix[2].z) * 2.0f;

      quat.w = (matrix[1].z - matrix[2].y) / s;
      quat.x = 0.25f * s;
      quat.y = (matrix[0].y + matrix[1].x) / s;
      quat.z = (matrix[0].z + matrix[2].x) / s;
   }
   else if (matrix[0].y > matrix[2].z) {
      const float s = sqrt(1.0f + matrix[1].y - matrix[0].x - matrix[2].z) * 2.0f;

      quat.w = (matrix[2].x - matrix[0].z) / s;
      quat.x = (matrix[0].y + matrix[1].x) / s;
      quat.y = 0.25f * s;
      quat.z = (matrix[1].z + matrix[2].y) / s;
   }
   else {
      const float s = sqrt(1.0f + matrix[2].z - matrix[0].x - matrix[1].y) * 2.0f;

      quat.w = (matrix[0].y - matrix[0].x) / s;
      quat.x = (matrix[0].z + matrix[2].x) / s;
      quat.y = (matrix[1].z + matrix[2].y) / s;
      quat.z = 0.25f * s;
   }

   return quat;
}

inline auto make_quat_from_matrix(const float4x4& matrix) noexcept -> quaternion
{
   return make_quat_from_matrix(float3x3{matrix});
}

inline auto to_matrix(const quaternion& quat) noexcept -> float4x4
{
   // Quaternion to Matrix Method from EuclideanSpace - https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
   const float ww = quat.w * quat.w;
   const float xx = quat.x * quat.x;
   const float yy = quat.y * quat.y;
   const float zz = quat.z * quat.z;

   const float xy = quat.x * quat.y;
   const float zw = quat.z * quat.w;

   const float xz = quat.x * quat.z;
   const float yw = quat.y * quat.w;

   const float yz = quat.y * quat.z;
   const float xw = quat.x * quat.w;

   const float inv_sq_len = 1.0f / (ww + xx + yy + zz);

   float4x4 m;

   m[0].x = (xx - yy - zz + ww) * inv_sq_len;
   m[1].x = 2.0f * (xy - zw) * inv_sq_len;
   m[2].x = 2.0f * (xz + yw) * inv_sq_len;

   m[0].y = 2.0f * (xy + zw) * inv_sq_len;
   m[1].y = (-xx + yy - zz + ww) * inv_sq_len;
   m[2].y = 2.0f * (yz - xw) * inv_sq_len;

   m[0].z = 2.0f * (xz - yw) * inv_sq_len;
   m[1].z = 2.0f * (yz + xw) * inv_sq_len;
   m[2].z = (-xx - yy + zz + ww) * inv_sq_len;

   return m;
}

inline auto make_quat_from_euler(const float3& vec) noexcept -> quaternion
{
   const float3 c = cos(vec * 0.5f);
   const float3 s = sin(vec * 0.5f);

   quaternion quat;
   quat.w = c.x * c.y * c.z + s.x * s.y * s.z;
   quat.x = s.x * c.y * c.z - c.x * s.y * s.z;
   quat.y = c.x * s.y * c.z + s.x * c.y * s.z;
   quat.z = c.x * c.y * s.z - s.x * s.y * c.z;

   return quat;
}

inline auto look_at_quat(const float3& to, const float3& from) -> quaternion
{
   const float3 direction = normalize(to - from);
   const float3 axis = cross(float3{0.0f, 0.0f, 1.0f}, direction);
   const float axis_dot = dot(float3{0.0f, 0.0f, 1.0f}, direction);

   quaternion quat{axis_dot + 1.0f, axis.x, axis.y, axis.z};

   return normalize(quat);
}

}