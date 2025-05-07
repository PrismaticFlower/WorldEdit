
#include "frame_constants.hlsli"
#include "lights_common.hlsli"
#include "block_material.hlsli"

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;
   nointerpolation uint material_index : MATERIAL;

   float4 positionSS : SV_Position;
};

// Followup: Normal Mapping Without Precomputed Tangents - Christian Schüler
// http://www.thetenthplanet.de/archives/1180
float3x3 cotangent_frame(const input_vertex input)
{
   const float3 normalWS = normalize(input.normalWS);

   const float3 dp1 = ddx(input.positionWS);
   const float3 dp2 = ddy(input.positionWS);

   const float2 duv1 = ddx(input.texcoords);
   const float2 duv2 = ddy(input.texcoords);

   const float3 dp2perp = cross(dp2, normalWS);
   const float3 dp1perp = cross(normalWS, dp1);

   const float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
   const float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

   const float inv_max = rsqrt(max(dot(T, T), dot(B, B)));

   return float3x3(T * inv_max, B * inv_max, normalWS);
}

float3 transform_normalWS(const input_vertex input, const float3 normalTS)
{
   return normalize(mul(normalTS, cotangent_frame(input)));
}

float4 main(input_vertex input) : SV_TARGET
{
   const float3 positionWS = input.positionWS;
   const float2 texcoords = input.texcoords;
   const float3 viewWS = normalize(cb_frame.view_positionWS - positionWS);

   block_material material = block_materials[input.material_index];

   Texture2D diffuse_map = Texture2DHeap[NonUniformResourceIndex(material.diffuse_map_index)];

   float4 diffuse_color = diffuse_map.Sample(sampler_anisotropic_wrap, texcoords);

   if (material.flags & block_material_has_detail_map) {
      Texture2D detail_map = Texture2DHeap[NonUniformResourceIndex(material.detail_map_index)];

      diffuse_color.rgb *=
         (detail_map.Sample(sampler_anisotropic_wrap, texcoords * material.detail_scale).rgb * 2.0);
   }
   
   float specular_visibility = diffuse_color.a;
   float3 normalWS;

   if (material.flags & block_material_has_normal_map) {
      Texture2D normal_map = Texture2DHeap[NonUniformResourceIndex(material.normal_map_index)];

      float4 normal_map_sample;

      if (material.flags & block_material_tile_normal_map) {
         normal_map_sample =
            normal_map.Sample(sampler_anisotropic_wrap, texcoords * material.detail_scale);
      }
      else {
         normal_map_sample = normal_map.Sample(sampler_anisotropic_wrap, texcoords);
      }

      const float3 normalTS = normal_map_sample.xyz * 2.0 - 1.0;
      normalWS = transform_normalWS(input, normalTS);
      specular_visibility = normal_map_sample.a;
   }
   else {
      normalWS = normalize(input.normalWS);
   }

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.viewWS = viewWS;
   lighting_inputs.diffuse_color = diffuse_color.rgb;
   lighting_inputs.specular_color = material.specular_color * specular_visibility;
   lighting_inputs.positionSS = input.positionSS.xy;
   lighting_inputs.receive_static_light = true;

   float3 lighting = calculate_lighting(lighting_inputs);

   if (material.flags & block_material_has_env_map) {
      TextureCube env_map = TextureCubeHeap[NonUniformResourceIndex(material.env_map_index)];

      const float3 reflectionWS = normalize(reflect(-viewWS, normalWS));

      lighting += env_map.Sample(sampler_anisotropic_wrap, reflectionWS).rgb * material.env_color *
                  specular_visibility;
   }

   return float4(lighting, 1.0);
}