#include "gizmo_rotation_widget_constants.hlsli"

#define NO_AA 0
#define AA_X4 1
#define AA_X8 0
#define AA_X256 0


// Modified version of Inigo Quilez's diskIntersect function.
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

float ring_intersect(float3 rd, float3 n)
{
   float outer_radius_sq = cb_gizmo.outer_radius_sq;
   float inner_radius_sq = cb_gizmo.inner_radius_sq;

   float3 o = -cb_gizmo.positionVS;
   float t = -dot(n, o) / dot(rd, n);
   float3 q = o + rd * t;

   float dist_sq = dot(q, q);
   return (dist_sq < cb_gizmo.outer_radius_sq && dist_sq > cb_gizmo.inner_radius_sq) ? t : -1.0;
}

float4 sample_gizmo(float3 ray_directionVS)
{
   float hit = 10e100;
   float4 color = 0.0;

   if (cb_gizmo.x_visible) {
      const float x_hit = ring_intersect(ray_directionVS, cb_gizmo.x_axisVS);

      if (x_hit >= 0.0) {
         hit = x_hit;
         color = float4(cb_gizmo.x_color, 1.0);
      }
   }

   if (cb_gizmo.y_visible) {
      const float y_hit = ring_intersect(ray_directionVS, cb_gizmo.y_axisVS);

      if (y_hit >= 0.0 && y_hit < hit) {
         hit = y_hit;
         color = float4(cb_gizmo.y_color, 1.0);
      }
   }

   if (cb_gizmo.z_visible) {
      const float z_hit = ring_intersect(ray_directionVS, cb_gizmo.z_axisVS);

      if (z_hit >= 0.0 && z_hit < hit) {
         hit = z_hit;
         color = float4(cb_gizmo.z_color, 1.0);
      }
   }

   return color;
}

float4 main(float3 ray_directionVS : RAYDIRVS) : SV_TARGET
{
#if NO_AA

   return sample_gizmo(normalize(ray_directionVS));

#elif AA_X4
   float4 color = 0.0;

   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(-2, -6))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(6, -2))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(-6, 2))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(2, 6))));

   return color * 0.25;
#elif AA_X8
   float4 color = 0.0;

   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(1, -3))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(-1, 3))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(5, 1))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(-3, -5))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(-5, 5))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(-7, -1))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(3, 7))));
   color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(7, -7))));

   return color * 0.125;
#elif AA_X256

   float4 color = 0.0;
   float samples = 0;

   for (int y = -8; y < 8; ++y) {
      for (int x = -8; x < 8; ++x) {
         color += sample_gizmo(normalize(EvaluateAttributeSnapped(ray_directionVS, int2(x, y))));
         samples += 1;
      }
   }

   return color / samples;
#endif
}
