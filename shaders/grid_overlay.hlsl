#include "frame_constants.hlsli"

struct output_vertex
{
   float positionWS_x : POSITIONWS_X;
   float positionWS_z : POSITIONWS_Z;
   float4 positionPS : SV_Position;
};

struct grid_constants {
   float3 grid_color;
   float length;
   float height;
   float inv_grid_scale;
   float line_width;
   float inv_grid_major_scale;
   float major_line_width;
};

ConstantBuffer<grid_constants> cb_grid : register(GRID_OVERLAY_CB_REGISTER);

output_vertex mainVS(uint vertex_index : SV_VertexID)
{
   float3 min_positionWS = {-cb_grid.length, 0.0, -cb_grid.length};
   float3 max_positionWS = {cb_grid.length, 0.0, cb_grid.length};
   
   float3 positionWS = 0.0;
   
   switch (vertex_index)
   {
      case 0:
         positionWS = float3(min_positionWS.x, cb_grid.height, min_positionWS.z);
         break;
      case 1:
         positionWS = float3(max_positionWS.x, cb_grid.height, min_positionWS.z);
         break;
      case 2:
         positionWS = float3(min_positionWS.x, cb_grid.height, max_positionWS.z);
         break;
      case 3:
         positionWS = float3(max_positionWS.x, cb_grid.height, max_positionWS.z);
         break;
      case 4:
         positionWS = float3(max_positionWS.x, cb_grid.height, min_positionWS.z);
         break;
      case 5:
         positionWS = float3(min_positionWS.x, cb_grid.height, max_positionWS.z);
         break;
   }
   
   positionWS.xz += cb_frame.view_positionWS.xz;
   
   output_vertex output;
   
   output.positionWS_x = positionWS.x;
   output.positionWS_z = positionWS.z;
   output.positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0));
   
   return output;
}

// Implmentation of Ben Golus's pristine grid shader: https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8#1e7c
// Minus the stuff for lines wider than 0.5 as we don't need it.
float eval_grid(const float2 uv, const float line_width)
{
   const float2 uv_deriv_x = ddx(uv);
   const float2 uv_deriv_y = ddy(uv);
   
   const float2 uv_deriv = float2(length(float2(uv_deriv_x.x, uv_deriv_y.x)), length(float2(uv_deriv_x.y, uv_deriv_y.y)));
   const float2 draw_width = clamp(line_width, uv_deriv, 0.5);
   const float2 line_aa = uv_deriv * 1.5;
   
   const float2 grid_uv = 1.0 - abs(frac(uv) * 2.0 - 1.0);
   float2 grid = smoothstep(draw_width + line_aa, draw_width - line_aa, grid_uv);
   
   grid *= saturate(line_width / draw_width);
   grid = lerp(grid, line_width, saturate(uv_deriv * 2.0 - 1.0));
   
   return lerp(grid.x, 1.0, grid.y);
}

[earlydepthstencil]
float4 mainPS(output_vertex input) : SV_Target0
{
   const float2 uv = float2(input.positionWS_x, input.positionWS_z);
   
   const float minor_alpha = eval_grid(uv * cb_grid.inv_grid_scale, cb_grid.line_width);
   const float major_alpha = eval_grid(uv * cb_grid.inv_grid_major_scale, cb_grid.major_line_width);
   const float alpha = lerp(minor_alpha, 1.0, major_alpha);
   
   if (alpha <= 1.0 / 512.0) discard;
   
   return float4(cb_grid.grid_color * alpha, alpha);
}
