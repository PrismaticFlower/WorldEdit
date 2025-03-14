#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct object_constants {
   float4x4 world_from_object;
};

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
      mul(cb_frame.projection_from_world, mul(object.world_from_object, float4(input.positionOS, 1.0)));

   return output;
}