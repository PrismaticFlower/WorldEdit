
#include "frame_constants.hlsli"
#include "lights_common.hlsli"
#include "material_normal.hlsli"
#include "samplers.hlsli"

enum flags {
   none = 0b0,
   transparent = 0b1,
   unlit = 0b10,
   specular_visibility_in_diffuse_map = 0b100,
   scrolling = 0b1000,
   static_lighting = 0b10000,
};

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMAL;
   float3 tangentWS : TANGENT;
   float3 bitangentWS : BITANGENT;
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;

   float4 positionSS : SV_Position;
};

float3 transform_normalWS(const input_vertex input, const float3 normalTS)
{
   return normalize(mul(normalTS, float3x3(input.tangentWS, input.bitangentWS, input.normalWS)));
}

float4 main(input_vertex input) : SV_TARGET
{
   Texture2D<float4> diffuse_map = ResourceDescriptorHeap[material.diffuse_map_index];
   Texture2D<float4> normal_map = ResourceDescriptorHeap[material.normal_map_index];

   float2 texcoords = input.texcoords;

   const float4 normal_map_sample = normal_map.Sample(sampler_anisotropic_wrap, texcoords);

   if (material.flags & flags::scrolling) {
      texcoords -= material.scrolling_amount * cb_frame.texture_scroll_duration;
   }

   const float3 normalTS = normal_map_sample.xyz * 2.0 - 1.0;
   const float3 normalWS = transform_normalWS(input, normalTS);

   const float3 positionWS = input.positionWS;
   const float3 viewWS = normalize(cb_frame.view_positionWS - positionWS);
   float4 diffuse_color = diffuse_map.Sample(sampler_anisotropic_wrap, texcoords);

   const bool static_lighting = material.flags & flags::static_lighting;

   if (static_lighting) {
      diffuse_color.a *= input.color.a;
   }
   else {
      diffuse_color *= input.color;
   }

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
   lighting_inputs.positionSS = input.positionSS.xy;
   lighting_inputs.receive_static_light = !static_lighting;

   float3 lighting = calculate_lighting(lighting_inputs);

   if (static_lighting) {
      lighting += input.color.rgb * diffuse_color.rgb;
   }

   return float4(lighting, diffuse_color.a);
}