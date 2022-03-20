
#include "bindless.hlsli"
#include "lights_common.hlsli"

enum flags { none = 0b0, transparent = 0b1, unlit = 0b10 };

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMAL;
   float3 tangentWS : TANGENT;
   float3 bitangentWS : BITANGENT;
   float2 texcoords : TEXCOORD;

   float4 positionSS : SV_Position;
};

struct material_input {
   uint flags;
   uint diffuse_map;
   uint normal_map;
};

ConstantBuffer<material_input> material : register(b0, space1);

SamplerState texture_sampler : register(s0);

float3 transform_normalWS(const input_vertex input, const float3 normalTS)
{
   return normalize(mul(normalTS, float3x3(input.tangentWS, input.bitangentWS, input.normalWS)));
}

float4 main(input_vertex input_vertex) : SV_TARGET
{
   Texture2D<float4> diffuse_map = GetTexture2D(material.diffuse_map);
   Texture2D<float4> normal_map = GetTexture2D(material.normal_map);

   const float3 normalTS = normal_map.Sample(texture_sampler, input_vertex.texcoords).xyz * 2.0 - 1.0;
   const float3 normalWS = transform_normalWS(input_vertex, normalTS);

   const float3 positionWS = input_vertex.positionWS;
   float4 diffuse_color = diffuse_map.Sample(texture_sampler, input_vertex.texcoords);

   if (material.flags & flags::transparent) diffuse_color.rgb *= diffuse_color.a;

   if (material.flags & flags::unlit) return diffuse_color;

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.diffuse_color = diffuse_color.rgb;
   lighting_inputs.positionSS = input_vertex.positionSS.xy;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, diffuse_color.a);
}