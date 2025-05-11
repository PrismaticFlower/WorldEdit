#pragma once

struct quaternion {
   float w;
   float x;
   float y;
   float z;
};

float3 mul(const quaternion quat, const float3 vec)
{
   const float3 u = {quat.x, quat.y, quat.z};
   const float s = quat.w;

   return 2.0 * dot(u, vec) * u + (s * s - dot(u, u)) * vec +
          (2.0 * s * cross(u, vec));
}