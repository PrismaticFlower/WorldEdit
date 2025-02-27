#include "bindings.hlsli"

struct camera_matrices {
   float4x4 projection_from_world;
};

struct object_constants {
   float4x4 world_from_object;
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
      mul(camera.projection_from_world, mul(object.world_from_object, float4(input.positionOS, 1.0)));

   return output;
}