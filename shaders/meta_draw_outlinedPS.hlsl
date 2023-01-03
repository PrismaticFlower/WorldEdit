#include "frame_constants.hlsli"

struct input_vertex {
   float4 positionRT : SV_Position;
   float4 color : COLOR;
   float4 outline_color : OUTLINE_COLOR;
   nointerpolation float4 positionPS : FLAT_POSITIONPS;
};

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

float line_distance(float2 p, float2 a, float2 b)
{
   return abs((b.y - a.y) * p.x - (b.x - a.x) * p.y + (b.x * a.y) - (b.y * a.x)) /
          sqrt(((b.y - a.y) * (b.y - a.y)) + ((b.x - a.x) * (b.x - a.x)));
}

float line_alpha(float distance)
{
   const float remapped_distance = clamp(distance - (cb_frame.line_width * 0.5 - 1.0), 0.0, 2.0);
   const float remapped_distance_sq = remapped_distance * remapped_distance;

   return exp2(-2.0 * remapped_distance_sq);
}

float4 main(input_vertex input) : SV_TARGET
{
   const float4 positionPS0 = GetAttributeAtVertex(input.positionPS, 0);
   const float4 positionPS1 = GetAttributeAtVertex(input.positionPS, 1);
   const float4 positionPS2 = GetAttributeAtVertex(input.positionPS, 2);

   const float2 positionRT0 = to_rendertarget_position(positionPS0);
   const float2 positionRT1 = to_rendertarget_position(positionPS1);
   const float2 positionRT2 = to_rendertarget_position(positionPS2);

   const float distance0 = line_distance(input.positionRT.xy, positionRT0, positionRT1);
   const float distance1 = line_distance(input.positionRT.xy, positionRT1, positionRT2);
   const float distance2 = line_distance(input.positionRT.xy, positionRT2, positionRT0);

   const float min_distance = min(min(distance0, distance1), distance2);

   return float4(lerp(float3(input.color.rgb), float3(input.outline_color.rgb), line_alpha(min_distance)),
                 1.0);
}