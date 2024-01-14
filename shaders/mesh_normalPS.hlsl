
#include "frame_constants.hlsli"
#include "lights_common.hlsli"
#include "material_normal.hlsli"
#include "resource_heaps.hlsli"
#include "samplers.hlsli"

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
   Texture2D diffuse_map = Texture2DHeap[material.diffuse_map_index];

   float2 texcoords = input.texcoords;

   if (material.flags & flags::scrolling) {
      texcoords -= material.scrolling_amount * cb_frame.texture_scroll_duration;
   }

   float4 normal_map_sample = float4(0.5, 0.5, 1.0, 1.0);

   if (material.flags & flags::has_normal_map) {
      Texture2D normal_map = Texture2DHeap[material.normal_map_index];

      if (material.flags & flags::tile_normal_map) {
         normal_map_sample =
            normal_map.Sample(sampler_anisotropic_wrap, texcoords * material.detail_scale);
      }
      else {
         normal_map_sample = normal_map.Sample(sampler_anisotropic_wrap, texcoords);
      }
   }

   const float3 normalTS = normal_map_sample.xyz * 2.0 - 1.0;
   const float3 normalWS = transform_normalWS(input, normalTS);

   const float3 positionWS = input.positionWS;
   const float3 viewWS = normalize(cb_frame.view_positionWS - positionWS);
   float4 diffuse_color = diffuse_map.Sample(sampler_anisotropic_wrap, texcoords);

   if (material.flags & flags::has_detail_map) {
      Texture2D detail_map = Texture2DHeap[material.detail_map_index];

      diffuse_color.rgb *=
         (detail_map.Sample(sampler_anisotropic_wrap, texcoords * material.detail_scale).rgb * 2.0);
   }

   const bool static_lighting = material.flags & flags::static_lighting;

   if (static_lighting) {
      diffuse_color.a *= input.color.a;
   }
   else {
      diffuse_color *= input.color;
   }

   if (material.flags & flags::transparent) diffuse_color.rgb *= diffuse_color.a;

   if (material.flags & flags::unlit) return diffuse_color;

   float specular_visibility = 1.0;

   if (material.flags & flags::specular_visibility_in_diffuse_map) {
      specular_visibility = diffuse_color.a;
   }
   else {
      specular_visibility = normal_map_sample.a;
   }

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.viewWS = viewWS;
   lighting_inputs.diffuse_color = diffuse_color.rgb;
   lighting_inputs.specular_color = material.specular_color * specular_visibility;
   lighting_inputs.positionSS = input.positionSS.xy;
   lighting_inputs.receive_static_light = !static_lighting;

   float3 lighting = calculate_lighting(lighting_inputs);

   if (static_lighting) {
      lighting += input.color.rgb * diffuse_color.rgb;
   }

   if (material.flags & flags::has_env_map) {
      TextureCube env_map = TextureCubeHeap[material.env_map_index];

      const float3 reflectionWS = normalize(reflect(-viewWS, normalWS));

      lighting += env_map.Sample(sampler_anisotropic_wrap, reflectionWS).rgb * material.env_color *
                  specular_visibility;
   }

   float alpha = diffuse_color.a;
   
   if (material.flags & flags::additive) alpha = 0.0;

   return float4(lighting, alpha);
}