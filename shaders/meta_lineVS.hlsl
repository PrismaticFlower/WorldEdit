#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct input_vertex {
   float3 positionWS : POSITION;
};

float4 main(input_vertex input) : SV_POSITION
{
   return mul(cb_frame.view_projection_matrix, float4(input.positionWS, 1.0));
}