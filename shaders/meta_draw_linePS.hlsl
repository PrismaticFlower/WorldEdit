#include "line_helpers.hlsli"

struct input_vertex {
   noperspective float line_distance : LINE_DISTANCE;
   nointerpolation float3 color : COLOR;
};

[earlydepthstencil] float4 main(input_vertex input) : SV_TARGET
{
   const float alpha = line_alpha(abs(input.line_distance));

   if (alpha == 0.0) discard;

   return float4(input.color, alpha);
}
