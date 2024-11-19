#include "gizmo_cone_constants.hlsli"

struct input_vertex {
   float3 ray_directionVS : RAYDIRVS;
};

const static float intersects_coverage = 0.25;

float dot2(float3 v)
{
   return dot(v, v);
}

struct cone_intersector {
   void init()
   {
      ra = cb_gizmo_cone.base_radius;
      ra_sq = ra * ra;

      ba = cb_gizmo_cone.endVS - cb_gizmo_cone.startVS;
      oa = -cb_gizmo_cone.startVS;
      ob = -cb_gizmo_cone.endVS;

      m0 = dot(ba, ba);
      m1 = dot(oa, ba);
      m5 = dot(oa, oa);

      m5 = dot(oa, oa);
      hy = m0 + ra_sq;
   }

   //  Modified version of Inigo Quilez's iCappedCone function.
   //
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
   float intersects(float3 rd)
   {
      float m3 = dot(rd, ba);

      // caps
      if (m1 < 0.0) {
         if (dot2(oa * m3 - rd * m1) < (ra_sq * m3 * m3)) {
            return intersects_coverage;
         }
      }

      // body
      float m4 = dot(rd, oa);

      float k2 = m0 * m0 - m3 * m3 * hy;
      float k1 = m0 * m0 * m4 - m1 * m3 * hy + m0 * ra * (ra * m3 * 1.0);
      float k0 = m0 * m0 * m5 - m1 * m1 * hy + m0 * ra * (ra * m1 * 2.0 - m0 * ra);

      float h = k1 * k1 - k2 * k0;
      if (h < 0.0) return 0.0;

      float t = (-k1 - sqrt(h)) / k2;

      float y = m1 + t * m3;
      if (y > 0.0 && y < m0) return intersects_coverage;


      return 0.0;
   }


   float ra;
   float ra_sq;

   float3 ba;
   float3 oa;
   float3 ob;

   float m0;
   float m1;
   
   float m5;
   float hy;
};

float4 main(input_vertex input) : SV_TARGET
{
   float coverage = 0.0;

   cone_intersector cone;
   cone.init();

   coverage += cone.intersects(normalize(EvaluateAttributeSnapped(input.ray_directionVS, int2(-2, -6))));
   coverage += cone.intersects(normalize(EvaluateAttributeSnapped(input.ray_directionVS, int2(6, -2))));
   coverage += cone.intersects(normalize(EvaluateAttributeSnapped(input.ray_directionVS, int2(-6, 2))));
   coverage += cone.intersects(normalize(EvaluateAttributeSnapped(input.ray_directionVS, int2(2, 6))));

   return float4(cb_gizmo_cone.color, coverage);
}
