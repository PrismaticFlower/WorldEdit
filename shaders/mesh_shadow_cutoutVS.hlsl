#include "bindings.hlsli"

struct camera_matrices {
   float4x4 view_projection_matrix;
};

struct object_constants {
   float4x4 world_matrix;
};

ConstantBuffer<camera_matrices> camera : register(FRAME_CB_REGISTER);
ConstantBuffer<object_constants> object : register(OBJECT_CB_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
   float3 normalOS : NORMAL;
   float3 tangentOS : TANGENT;
   float3 bitangentOS : BITANGENT;
   float2 texcoords : TEXCOORD;
};

struct output_vertex {
   float2 texcoords : TEXCOORD;
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   output.texcoords = input.texcoords;
   output.positionPS =
      mul(camera.view_projection_matrix, mul(object.world_matrix, float4(input.positionOS, 1.0)));
   output.positionPS.z = max(output.positionPS.z, 0.0);

   return output;
}