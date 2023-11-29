
#include "frame_constants.hlsli"
#include "lights_common.hlsli"
#include "terrain_common.hlsli"

const static float3 surface_color = 0.75;

float4 main(input_vertex input) : SV_Target0
{
   const float3 normalWS = get_terrain_normalWS(input.terrain_coords);

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = input.positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.viewWS = normalize(cb_frame.view_positionWS - input.positionWS);
   lighting_inputs.diffuse_color = surface_color * input.color;
   lighting_inputs.specular_color = 0.0;
   lighting_inputs.positionSS = input.positionSS.xy;
   lighting_inputs.receive_static_light = true;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, 1.0f);
}