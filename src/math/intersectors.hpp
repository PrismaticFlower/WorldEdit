#pragma once

#include "types.hpp"

namespace we {

// The MIT License
// Copyright © Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to
// do so, subject to the following conditions: The above copyright notice and this
// permission notice shall be included in all copies or substantial portions of
// the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

inline float cross2d(float2 a, float2 b)
{
   return a.x * b.y - a.y * b.x;
}

inline float dot2(float3 v)
{
   return dot(v, v);
}

inline float sphIntersect(float3 ro, float3 rd, float3 sph, float radius)
{
   float3 oc = ro - sph;
   float b = glm::dot(oc, rd);
   float c = glm::dot(oc, oc) - radius * radius;
   float h = b * b - c;
   if (h < 0.0f) return -1.0f;
   return -b - sqrt(h);
}

inline float2 boxIntersection(float3 ro, float3 rd, float3 boxSize, float3& outNormal)
{
   float3 m = 1.0f / rd; // can precompute if traversing a set of aligned boxes
   float3 n = m * ro;    // can precompute if traversing a set of aligned boxes
   float3 k = abs(m) * boxSize;
   float3 t1 = -n - k;
   float3 t2 = -n + k;
   float tN = glm::max(glm::max(t1.x, t1.y), t1.z);
   float tF = glm::min(glm::min(t2.x, t2.y), t2.z);
   if (tN > tF || tF < 0.0f) return float2(-1.0f); // no intersection
   outNormal = -sign(rd) * step(float3{t1.y, t1.z, t1.x}, float3{t1.x, t1.y, t1.z}) *
               step(float3{t1.z, t1.x, t1.y}, float3{t1.x, t1.y, t1.z});
   return float2(tN, tF);
}

inline float3 quadIntersect(float3 ro, float3 rd, float3 v0, float3 v1,
                            float3 v2, float3 v3)
{
   const int lut[4] = {1, 2, 0, 1};

   // lets make v0 the origin
   float3 a = v1 - v0;
   float3 b = v3 - v0;
   float3 c = v2 - v0;
   float3 p = ro - v0;

   // intersect plane
   float3 nor = cross(a, b);
   float t = -dot(p, nor) / dot(rd, nor);
   if (t < 0.0) return float3(-1.0);

   // intersection point
   float3 pos = p + t * rd;

   // see here: https://www.shadertoy.com/view/lsBSDm

   // select projection plane
   float3 mor = abs(nor);
   int id = (mor.x > mor.y && mor.x > mor.z) ? 0 : (mor.y > mor.z) ? 1 : 2;

   int idu = lut[id];
   int idv = lut[id + 1];

   // project to 2D
   float2 kp = float2(pos[idu], pos[idv]);
   float2 ka = float2(a[idu], a[idv]);
   float2 kb = float2(b[idu], b[idv]);
   float2 kc = float2(c[idu], c[idv]);

   // find barycentric coords of the quadrilateral
   float2 kg = kc - kb - ka;

   float k0 = cross2d(kp, kb);
   float k2 = cross2d(kc - kb, ka); // float k2 = cross2d( kg, ka );
   float k1 = cross2d(kp, kg) -
              nor[id]; // float k1 = cross2d( kb, ka ) + cross2d( kp, kg );

   // if edges are parallel, this is a linear equation
   float u, v;
   if (abs(k2) < 0.00001) {
      v = -k0 / k1;
      u = cross2d(kp, ka) / k1;
   }
   else {
      // otherwise, it's a quadratic
      float w = k1 * k1 - 4.0f * k0 * k2;
      if (w < 0.0f) return float3(-1.0f);
      w = glm::sqrt(w);

      float ik2 = 1.0f / (2.0f * k2);

      v = (-k1 - w) * ik2;
      if (v < 0.0f || v > 1.0f) v = (-k1 + w) * ik2;

      u = (kp.x - ka.x * v) / (kb.x + kg.x * v);
   }

   if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) return float3(-1.0f);

   return float3(t, u, v);
}

inline float4 iCappedCone(float3 ro, float3 rd, float3 pa, float3 pb, float ra, float rb)
{
   float3 ba = pb - pa;
   float3 oa = ro - pa;
   float3 ob = ro - pb;

   float m0 = dot(ba, ba);
   float m1 = dot(oa, ba);
   float m2 = dot(ob, ba);
   float m3 = dot(rd, ba);

   // caps
   if (m1 < 0.0f) {
      if (dot2(oa * m3 - rd * m1) < (ra * ra * m3 * m3))
         return float4(-m1 / m3, -ba * glm::inversesqrt(m0));
   }
   else if (m2 > 0.0f) {
      if (dot2(ob * m3 - rd * m2) < (rb * rb * m3 * m3))
         return float4(-m2 / m3, ba * glm::inversesqrt(m0));
   }

   // body
   float m4 = dot(rd, oa);
   float m5 = dot(oa, oa);
   float rr = ra - rb;
   float hy = m0 + rr * rr;

   float k2 = m0 * m0 - m3 * m3 * hy;
   float k1 = m0 * m0 * m4 - m1 * m3 * hy + m0 * ra * (rr * m3 * 1.0f);
   float k0 = m0 * m0 * m5 - m1 * m1 * hy + m0 * ra * (rr * m1 * 2.0f - m0 * ra);

   float h = k1 * k1 - k2 * k0;
   if (h < 0.0f) return float4(-1.0f);

   float t = (-k1 - sqrt(h)) / k2;

   float y = m1 + t * m3;
   if (y > 0.0 && y < m0) {
      return float4(t, normalize(m0 * (m0 * (oa + t * rd) + rr * ba * ra) - ba * hy * y));
   }

   return float4(-1.0f);
}

inline float4 iCylinder(float3 ro, float3 rd, float3 pa, float3 pb,
                        float ra) // extreme a, extreme b, radius
{
   float3 ba = pb - pa;

   float3 oc = ro - pa;

   float baba = dot(ba, ba);
   float bard = dot(ba, rd);
   float baoc = dot(ba, oc);

   float k2 = baba - bard * bard;
   float k1 = baba * dot(oc, rd) - baoc * bard;
   float k0 = baba * dot(oc, oc) - baoc * baoc - ra * ra * baba;

   float h = k1 * k1 - k2 * k0;
   if (h < 0.0f) return float4(-1.0f);
   h = sqrt(h);
   float t = (-k1 - h) / k2;

   // body
   float y = baoc + t * bard;
   if (y > 0.0 && y < baba)
      return float4(t, (oc + t * rd - ba * y / baba) / ra);

   // caps
   t = (((y < 0.0f) ? 0.0f : baba) - baoc) / bard;
   if (abs(k1 + k2 * t) < h) {
      return float4(t, ba * glm::sign(y) / baba);
   }

   return float4(-1.0);
}

inline float3 triIntersect(float3 ro, float3 rd, float3 v0, float3 v1, float3 v2)
{
   float3 v1v0 = v1 - v0;
   float3 v2v0 = v2 - v0;
   float3 rov0 = ro - v0;
   float3 n = cross(v1v0, v2v0);
   float3 q = cross(rov0, rd);
   float d = 1.0f / dot(rd, n);
   float u = d * dot(-q, v2v0);
   float v = d * dot(q, v1v0);
   float t = d * dot(-n, rov0);
   if (u < 0.0f || v < 0.0f || (u + v) > 1.0f) t = -1.0f;
   return float3(t, u, v);
}

}