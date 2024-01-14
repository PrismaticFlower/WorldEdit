
#include "frame_constants.hlsli"
#include "lights_common.hlsli"
#include "resource_heaps.hlsli"
#include "terrain_common.hlsli"

float4 main(input_vertex input) : SV_Target0
{
   const float3 normalWS = get_terrain_normalWS(input.terrain_coords);
   const float3 positionWS = input.positionWS;
   const float3 viewWS = normalize(cb_frame.view_positionWS - positionWS);

   Texture2D base_diffuse_map = Texture2DHeap[terrain_constants.diffuse_maps_index[0]];

   float3 diffuse_color = 
      base_diffuse_map.Sample(sampler_anisotropic_wrap, get_terrain_texcoords(0, positionWS)).rgb;

   const uint active_textures = input.active_textures;

   for (uint i = 1; i < terrain_max_textures; ++i) {
      [branch] if (!input.active_textures & (1u << i)) continue;

      const float weight =
         texture_weight_maps.Sample(sampler_bilinear_clamp, float3(input.terrain_coords, i)).r;

      const float2 texcoords = get_terrain_texcoords(i, positionWS);

      Texture2D diffuse_map = Texture2DHeap[terrain_constants.diffuse_maps_index[i]];

      diffuse_color = lerp(diffuse_color, diffuse_map.Sample(sampler_anisotropic_wrap, texcoords).rgb, weight);
   }

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.viewWS = viewWS;
   lighting_inputs.diffuse_color = input.color * diffuse_color;
   lighting_inputs.specular_color = 0.0;
   lighting_inputs.positionSS = input.positionSS.xy;
   lighting_inputs.receive_static_light = true;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, 1.0);
}