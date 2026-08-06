#include "bindings.hlsli"
#include "frame_constants.hlsli"
#include "fog.hlsli"
#include "lights_common.hlsli"

struct input_vertex {
   int4   positionOS : POSITION;
   int2   texcoords  : TEXCOORD;
   float3x4 world_from_object : TRANSFORM;
};

struct output_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS  : NORMALWS;
   float2 texcoords : TEXCOORD;
   float  fog : FOG;
   float  darkening : DARKNESS;

   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionOS = input.positionOS * (1.0 / 655.35);
   const float3 positionWS = mul(input.world_from_object, float4(positionOS, 1.0)).xyz;
   const float4 positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));;
   
   output.positionWS = positionWS;
   output.normalWS = mul((float3x3)input.world_from_object, normalize(positionOS));
   output.texcoords = input.texcoords * (1.0 / 2048.0);
   output.fog = calculate_fog(positionWS, positionPS);
   output.darkening = input.positionOS.w * (1.0 / 255.0);
   output.positionPS = positionPS;

   return output;
}