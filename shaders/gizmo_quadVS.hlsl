#include "frame_constants.hlsli"
#include "gizmo_quad_constants.hlsli"

struct output_vertex {
   float2 rect_coords : RECTCOORDS;
   float outer_edge : OUTER;
   float4 positionPS : SV_Position;
};

output_vertex main(uint vertex_index : SV_VertexID)
{
   output_vertex output;

   float3 positionWS = 0.0;
   float2 rect_coords = 0.0;
   float outer_edge = 0.0;

   switch (vertex_index)
   {
      case 0:
         positionWS = cb_gizmo_quad.corner0WS;
         rect_coords = float2(-0.5, -0.5);
         break;
      case 1:
         positionWS = cb_gizmo_quad.corner1WS;
         outer_edge = 1.0;
         rect_coords = float2(0.5, -0.5);
         break;
      case 2:
         positionWS = cb_gizmo_quad.corner2WS;
         rect_coords = float2(0.5, 0.5);
         break;
      case 3:
         positionWS = cb_gizmo_quad.corner3WS;
         outer_edge = 1.0;
         rect_coords = float2(-0.5, 0.5);
         break;
      case 4:
         positionWS = cb_gizmo_quad.corner0WS;
         rect_coords = float2(-0.5, -0.5);
         break;
      case 5:
         positionWS = cb_gizmo_quad.corner2WS;
         rect_coords = float2(0.5, 0.5);
         break;
   }

   output.rect_coords = rect_coords;
   output.outer_edge = outer_edge;
   output.positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0));

   return output;
}