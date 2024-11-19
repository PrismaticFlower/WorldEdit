#include "gizmo_line_constants.hlsli"

struct input_vertex {
   float outer_edge : OUTER;
   uint coverage : SV_Coverage;
};

float4 main(input_vertex input) : SV_TARGET
{
   return float4(cb_gizmo_line.color, input.outer_edge > 0.0 ? countbits(input.coverage) * 0.0625 : 1.0);
}
