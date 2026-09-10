#include "bindings.hlsli"
#include "frame_constants.hlsli"
#include "fog.hlsli"

struct input_vertex {
   int4   positionOS : POSITION;
   float4 color   : COLOR;
   int2   texcoords  : TEXCOORD;
   float3x4 world_from_object : TRANSFORM;
};

struct output_vertex {
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;
   float  fog : FOG;

   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionOS = input.positionOS * (1.0 / 81.9175);
   const float3 positionWS = mul(input.world_from_object, float4(positionOS, 1.0)).xyz;
   const float4 positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));;
   
   output.texcoords = input.texcoords * (1.0 / 2048.0);
   output.fog = calculate_fog(positionWS, positionPS);
   output.color = input.color;
   output.positionPS = positionPS;

   return output;
}