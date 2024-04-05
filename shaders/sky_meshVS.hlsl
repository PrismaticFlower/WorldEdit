#include "bindings.hlsli"
#include "frame_constants.hlsli"
#include "sky_mesh_constants.hlsli"
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

   float3 positionOS = mul(cb_mesh_constants.rotation, float4(input.positionOS, 1.0)).xyz;

   float3 view_positionWS = cb_frame.view_positionWS;
   float3 positionWS = positionOS + view_positionWS * float3(1.0, cb_mesh_constants.movement_scale, 1.0);

   positionWS.y += ((1.0 - cb_mesh_constants.movement_scale) * cb_mesh_constants.offset);

   output.texcoords = input.texcoords;
   output.color = srgb_to_linear(input.color);
   output.positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0));
   output.positionPS.z = 0.0;

   return output;
}