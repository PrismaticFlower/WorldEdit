#include "frame_constants.hlsli"
#include "gizmo_line_constants.hlsli"

struct output_vertex {
   float outer_edge : OUTER;
   float4 positionPS : SV_Position;
};

output_vertex main(uint vertex_index : SV_VertexID)
{
   output_vertex output;

   float3 positionVS = 0.0;
   float outer_edge = 0.0;

   switch (vertex_index)
   {
      case 0:
         positionVS = cb_gizmo_line.endVS - cb_gizmo_line.extensionVS;
         break;
      case 1:
         positionVS = cb_gizmo_line.startVS - cb_gizmo_line.extensionVS;
         outer_edge = 1.0;
         break;
      case 2:
         positionVS = cb_gizmo_line.startVS + cb_gizmo_line.extensionVS;
         break;
      case 3:
         positionVS = cb_gizmo_line.endVS - cb_gizmo_line.extensionVS;
         break;
      case 4:
         positionVS = cb_gizmo_line.startVS + cb_gizmo_line.extensionVS;
         break;
      case 5:
         positionVS = cb_gizmo_line.endVS + cb_gizmo_line.extensionVS;
         outer_edge = 1.0;
         break;
   }

   output.outer_edge = outer_edge;
   output.positionPS = mul(cb_frame.projection_matrix, float4(positionVS, 1.0));

   return output;
}