#include "bindings.hlsli"
#include "srgb.hlsli"

struct camera_constants {
   float4x4 projection_from_world;
};

ConstantBuffer<camera_constants> cb_camera : register(THUMBNAIL_CAMERA_CB_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
   float3 normalOS : NORMAL;
   float3 tangentOS : TANGENT;
   float3 bitangentOS : BITANGENT;
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;
};

struct output_vertex {
   float3 positionOS : POSITION;
   float3 normalOS : NORMAL;
   float3 tangentOS : TANGENT;
   float3 bitangentOS : BITANGENT;
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;

   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;
   
   output.positionOS = input.positionOS;
   output.normalOS = input.normalOS;
   output.tangentOS = input.tangentOS;
   output.bitangentOS = input.bitangentOS;
   output.texcoords = input.texcoords;
   output.color = srgb_to_linear(input.color);
   output.positionPS = mul(cb_camera.projection_from_world, float4(input.positionOS, 1.0));

   return output;
}