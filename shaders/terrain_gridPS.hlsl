
#include "frame_constants.hlsli"
#include "terrain_common.hlsli"

[earlydepthstencil]
float4 main(input_vertex input) : SV_Target0
{
   const float2 uv = input.positionWS.xz * terrain_constants.inv_grid_size;
   const float3 color = terrain_constants.grid_line_color;
   
   // Implmentation of Ben Golus's pristine grid shader: https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8#1e7c
   // Minus the stuff for lines wider than 0.5 as we don't need it.

   const float line_width = terrain_constants.grid_line_width;
   const float2 uv_deriv_x = ddx(uv);
   const float2 uv_deriv_y = ddy(uv);
   
   const float2 uv_deriv = float2(length(float2(uv_deriv_x.x, uv_deriv_y.x)), length(float2(uv_deriv_x.y, uv_deriv_y.y)));
   const float2 draw_width = clamp(line_width, uv_deriv, 0.5);
   const float2 line_aa = uv_deriv * 1.5;
   
   const float2 grid_uv = 1.0 - abs(frac(uv) * 2.0 - 1.0);
   float2 grid = smoothstep(draw_width + line_aa, draw_width - line_aa, grid_uv);
   
   grid *= saturate(line_width / draw_width);
   grid = lerp(grid, line_width, saturate(uv_deriv * 2.0 - 1.0));
   
   const float alpha = lerp(grid.x, 1.0, grid.y);
   
   if (alpha <= 1.0 / 512.0) discard;
   
   return float4(color * alpha, alpha);
}