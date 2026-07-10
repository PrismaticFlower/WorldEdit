
#include "frame_constants.hlsli"
#include "fog.hlsli"
#include "lights_common.hlsli"
#include "resource_heaps.hlsli"
#include "terrain_common.hlsli"

#define WE_TERRAIN_SKIP_0_WEIGHT_TEXTURES

float4 main(vertex input) : SV_Target0
{
   const float3 positionWS = input.positionWS;

   float3 diffuse_color = 0.0;

   [unroll]
   for (uint y = 0; y < TERRAIN_MAX_TEXTURES / 4; ++y) {
      [unroll]
      for (uint x = 0; x < TERRAIN_MAX_TEXTURES / 4; ++x) {
         const float weight = input.weights[y][x];
         const float2 texcoords = get_terrain_texcoords(y * 4 + x, positionWS);

         Texture2D diffuse_map = Texture2DHeap[terrain_constants.diffuse_maps_index[y][x]];

         float3 layer_color = 0.0;

         #ifdef WE_TERRAIN_SKIP_0_WEIGHT_TEXTURES
            if (WaveActiveAnyTrue(weight > 0.0)) {
               layer_color = diffuse_map.Sample(sampler_anisotropic_wrap, texcoords).rgb;
            }
         #else
            layer_color = diffuse_map.Sample(sampler_anisotropic_wrap, texcoords).rgb;
         #endif

         diffuse_color = lerp(diffuse_color, layer_color, weight);
      }
   }

   if (terrain_constants.has_detail_map) {
      Texture2D detail_map = Texture2DHeap[terrain_constants.detail_map_index];

      diffuse_color *= 2.0 * detail_map.Sample(sampler_anisotropic_wrap, positionWS.xz * 0.912).rgb;
      diffuse_color *= 2.0 * detail_map.Sample(sampler_anisotropic_wrap, positionWS.xz * 1.72).rgb;
      diffuse_color *= 2.0 * detail_map.Sample(sampler_anisotropic_wrap, positionWS.xz * 3.2).rgb;
   }

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = input.normalWS;
   lighting_inputs.viewWS = normalize(cb_frame.view_positionWS - positionWS);
   lighting_inputs.diffuse_color = diffuse_color;
   lighting_inputs.specular_color = 0.0;
   lighting_inputs.positionSS = input.positionPS.xy;
   lighting_inputs.receive_static_light = false;

   float3 lighting = calculate_lighting(lighting_inputs);
   lighting += (input.static_light * diffuse_color);

   return apply_fog(float4(lighting, 1.0), input.fog);
}