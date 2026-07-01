
#include "frame_constants.hlsli"
#include "fog.hlsli"
#include "lights_common.hlsli"
#include "terrain_common.hlsli"

const static float3 surface_color = 0.75;

float4 main(vertex input) : SV_Target0
{
   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = input.positionWS;
   lighting_inputs.normalWS = input.normalWS;
   lighting_inputs.viewWS = normalize(cb_frame.view_positionWS - input.positionWS);
   lighting_inputs.diffuse_color = surface_color;
   lighting_inputs.specular_color = 0.0;
   lighting_inputs.positionSS = input.positionPS.xy;
   lighting_inputs.receive_static_light = false;

   const float3 lighting = calculate_lighting(lighting_inputs) + input.static_light;

   return apply_fog(float4(lighting, 1.0f), input.fog);
}