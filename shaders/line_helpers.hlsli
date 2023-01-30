#pragma once

#include "frame_constants.hlsli"

float2 to_rendertarget_position(float4 positionPS)
{
   const float2 positionNDC = positionPS.xy / positionPS.w;
   const float2 viewport_size = cb_frame.viewport_size;
   const float2 viewport_topleft = cb_frame.viewport_topleft;

   float2 positionRT;

   positionRT.x = (positionNDC.x + 1.0) * viewport_size.x * 0.5 + viewport_topleft.x;
   positionRT.y = (1.0 - positionNDC.y) * viewport_size.y * 0.5 + viewport_topleft.y;

   return positionRT;
}

float4 from_rendertarget_position(float2 positionRT, float4 ref_positionPS)
{
   const float2 viewport_size = cb_frame.viewport_size;
   const float2 viewport_topleft = cb_frame.viewport_topleft;

   float2 positionVP = (positionRT - viewport_topleft) / viewport_size;
   float2 positionNDC = (positionVP - 0.5) * float2(2.0, -2.0);

   return float4(positionNDC * ref_positionPS.w, ref_positionPS.zw);
}

float line_distance(float2 p, float2 a, float2 b)
{
   return abs((b.y - a.y) * p.x - (b.x - a.x) * p.y + (b.x * a.y) - (b.y * a.x)) /
          sqrt(((b.y - a.y) * (b.y - a.y)) + ((b.x - a.x) * (b.x - a.x)));
}

float line_alpha(float distance)
{
   const float remapped_distance = clamp(distance - (cb_frame.line_width * 0.5 - 1.0), 0.0, 2.0);
   const float remapped_distance_sq = remapped_distance * remapped_distance;
   const float alpha = exp2(-2.0 * remapped_distance_sq);

   return alpha - (1.0 / 255.0) * (1.0 + (1.0 / 255.0)); // remap into 0..1 range
}
