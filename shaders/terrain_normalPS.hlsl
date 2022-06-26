
#include "frame_constants.hlsli"
#include "lights_common.hlsli"
#include "terrain_common.hlsli"

ConstantBuffer<frame_constant_buffer> cb_frame : register(b0, space0);

Texture2D<float3> diffuse_maps[TERRAIN_MAX_TEXTURES] : register(t0, space1);

float4 main(input_vertex input) : SV_Target0
{
   const float3 normalWS = get_terrain_normalWS(input.terrain_coords);
   const float3 positionWS = input.positionWS;
   const float3 viewWS = normalize(cb_frame.view_positionWS - positionWS);

   float3 diffuse_color = 0.0;

   const uint active_textures = input.active_textures;
   float total_weight = 0.0;

   for (uint i = 0; i < terrain_max_textures; ++i) {
      [branch] if (!input.active_textures & (1u << i)) continue;

      const float weight =
         texture_weight_maps.Sample(bilinear_clamp_sampler, float3(input.terrain_coords, i));

      const float2 texcoords = get_terrain_texcoords(i, positionWS);

      diffuse_color += weight * diffuse_maps[i].Sample(trilinear_sampler, texcoords);
      total_weight += weight;
   }

   diffuse_color /= total_weight;

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.viewWS = viewWS;
   lighting_inputs.diffuse_color = diffuse_color;
   lighting_inputs.specular_color = 0.0;
   lighting_inputs.positionSS = input.positionSS.xy;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, 1.0);
}