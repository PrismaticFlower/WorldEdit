
#include "frame_constants.hlsli"
#include "lights_common.hlsli"
#include "material_normal.hlsli"
#include "samplers.hlsli"

enum flags {
   none = 0b0,
   transparent = 0b1,
   unlit = 0b10,
   specular_visibility_in_diffuse_map = 0b100
};

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMAL;
   float3 tangentWS : TANGENT;
   float3 bitangentWS : BITANGENT;
   float2 texcoords : TEXCOORD;

   float4 positionSS : SV_Position;
};

float3 transform_normalWS(const input_vertex input, const float3 normalTS)
{
   return normalize(mul(normalTS, float3x3(input.tangentWS, input.bitangentWS, input.normalWS)));
}

float4 main(input_vertex input_vertex) : SV_TARGET
{
   Texture2D<float4> diffuse_map = ResourceDescriptorHeap[material.diffuse_map_index];
   Texture2D<float4> normal_map = ResourceDescriptorHeap[material.normal_map_index];

   const float4 normal_map_sample = normal_map.Sample(sampler_anisotropic_wrap, input_vertex.texcoords);

   const float3 normalTS = normal_map_sample.xyz * 2.0 - 1.0;
   const float3 normalWS = transform_normalWS(input_vertex, normalTS);

   const float3 positionWS = input_vertex.positionWS;
   const float3 viewWS = normalize(cb_frame.view_positionWS - positionWS);
   float4 diffuse_color = diffuse_map.Sample(sampler_anisotropic_wrap, input_vertex.texcoords);

   if (material.flags & flags::transparent) diffuse_color.rgb *= diffuse_color.a;

   if (material.flags & flags::unlit) return diffuse_color;

   float3 specular_color = material.specular_color;

   if (material.flags & flags::specular_visibility_in_diffuse_map) {
      specular_color *= diffuse_color.a;
   }
   else {
      specular_color *= normal_map_sample.a;
   }

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.viewWS = viewWS;
   lighting_inputs.diffuse_color = diffuse_color.rgb;
   lighting_inputs.specular_color = specular_color;
   lighting_inputs.positionSS = input_vertex.positionSS.xy;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, diffuse_color.a);
}