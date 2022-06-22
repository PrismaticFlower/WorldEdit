#include "frame_constants.hlsli"

ConstantBuffer<frame_constant_buffer> cb_frame : register(b0);

struct input_vertex {
   float3 positionWS : POSITION;
};

float4 main(input_vertex input) : SV_POSITION
{
   return mul(cb_frame.view_projection_matrix, float4(input.positionWS, 1.0));
}