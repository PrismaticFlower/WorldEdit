#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct object_constants {
   float4x4 world_matrix;
};

ConstantBuffer<object_constants> cb_object_constants : register(OBJECT_CB_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
};

struct output_vertex {
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionWS = mul(cb_object_constants.world_matrix, float4(input.positionOS, 1.0)).xyz;

   output.positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0));

   return output;
}