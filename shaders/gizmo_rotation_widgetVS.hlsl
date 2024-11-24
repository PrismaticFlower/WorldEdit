#include "frame_constants.hlsli"
#include "gizmo_rotation_widget_constants.hlsli"

struct input_vertex {
   float3 positionOS : POSITION;
};

struct output_vertex {
   float3 ray_directionVS : RAYDIRVS;

   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionVS = input.positionOS * cb_gizmo.bbox_scaleVS + cb_gizmo.bbox_positionVS;

   output.ray_directionVS = positionVS;
   output.positionPS = mul(cb_frame.projection_matrix, float4(positionVS, 1.0));

   return output;
}