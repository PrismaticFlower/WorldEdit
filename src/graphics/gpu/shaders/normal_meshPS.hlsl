
#include "lights_common.hlsli"

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;
};

Texture2D<float4> diffuse_map : register(t0, space1);

SamplerState trilinear_sampler : register(s0);

float4 main(input_vertex input_vertex) : SV_TARGET
{
   const float3 normalWS = normalize(input_vertex.normalWS);
   const float3 positionWS = input_vertex.positionWS;
   const float3 diffuse_color =
      diffuse_map.Sample(trilinear_sampler, input_vertex.texcoords).rgb;

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.diffuse_color = diffuse_color;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, 1.0f);
}