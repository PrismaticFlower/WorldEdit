#include "gizmo_quad_constants.hlsli"

struct input_vertex {
   float2 rect_coords : RECTCOORDS;
   float outer_edge : OUTER;
   uint coverage : SV_Coverage;
};

float inner_rect(float2 coords)
{
   const float aa_scale = 1.5;

   float2 distance2 = abs(coords) - cb_gizmo_quad.outline_width;
   float distance = max(distance2.x, distance2.y);

   float width = 0.5 * fwidth(distance) * aa_scale;

   return 1.0 - smoothstep(-width, width, distance);
}

float4 main(input_vertex input) : SV_TARGET
{
   float alpha = lerp(1.0, cb_gizmo_quad.inner_alpha, inner_rect(input.rect_coords));

   float coverage_alpha; 
   
   if (input.outer_edge > 0.0) {
      coverage_alpha = countbits(input.coverage) * 0.0625;
   }
   else if (alpha < 1.0 && countbits(input.coverage) != 16) {
      coverage_alpha = 0.0;
   }
   else {
      coverage_alpha = 1.0;
   }
   
   return float4(cb_gizmo_quad.color, min(alpha, coverage_alpha));
}

float4 main_no_aa(input_vertex input) : SV_TARGET
{
   input.coverage = 0xffff;
   
   return main(input);
}