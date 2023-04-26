#include "bindings.hlsli"
#include "frame_constants.hlsli"
#include "srgb.hlsli"

struct input_vertex {
   float3 positionOS : POSITION;
   float3 normalOS : NORMAL;
   float3 tangentOS : TANGENT;
   float3 bitangentOS : BITANGENT;
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;
};

struct output_vertex {
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;

   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionWS = input.positionOS + cb_frame.view_positionWS;

   output.texcoords = input.texcoords;
   output.color = srgb_to_linear(input.color);
   output.positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0)).xyww;

   return output;
}