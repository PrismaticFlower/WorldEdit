#include "line_helpers.hlsli"

struct input_vertex {
   float4 positionRT : SV_Position;
   float4 color : COLOR;
   float4 outline_color : OUTLINE_COLOR;
   nointerpolation float4 positionPS : FLAT_POSITIONPS;
};

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
