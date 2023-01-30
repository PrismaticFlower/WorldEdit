#include "line_helpers.hlsli"

struct input_vertex {
   noperspective float line_distance : LINE_DISTANCE;
   nointerpolation float3 color : COLOR;
};

float4 main(input_vertex input) : SV_TARGET
{
   return float4(input.color, line_alpha(abs(input.line_distance)));
}
