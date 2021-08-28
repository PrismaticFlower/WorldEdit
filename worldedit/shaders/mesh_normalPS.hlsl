
#include "lights_common.hlsli"

namespace material {
const static bool transparent = MATERIAL_TRANSPARENT;
}

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMAL;
   float3 tangentWS : TANGENT;
   float3 bitangentWS : BITANGENT;
   float2 texcoords : TEXCOORD;
};

Texture2D<float4> diffuse_map : register(t0, space1);
Texture2D<float4> normal_map : register(t1, space1);

SamplerState trilinear_sampler : register(s0);

float3 transform_normalWS(const input_vertex input, const float3 normalTS)
{
   return normalize(mul(normalTS, float3x3(input.tangentWS, input.bitangentWS, input.normalWS)));
}

float4 main(input_vertex input_vertex) : SV_TARGET
{
   const float3 normalTS = normal_map.Sample(trilinear_sampler, input_vertex.texcoords).xyz * 2.0 - 1.0;
   const float3 normalWS = transform_normalWS(input_vertex, normalTS);

   const float3 positionWS = input_vertex.positionWS;
   float4 diffuse_color = diffuse_map.Sample(trilinear_sampler, input_vertex.texcoords);

   if (material::transparent) diffuse_color.rgb *= diffuse_color.a;

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.diffuse_color = diffuse_color.rgb;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, material::transparent ? diffuse_color.a : 1.0);
}