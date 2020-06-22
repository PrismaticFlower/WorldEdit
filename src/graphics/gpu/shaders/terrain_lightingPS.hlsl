
#include "lights_common.hlsli"
#include "terrain_common.hlsli"

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float2 terrain_coords : TERRAINCOORDS;
};

const static float3 surface_color = 0.75;

float4 main(input_vertex input) : SV_Target0
{
   const float3 normalWS = get_terrain_normalWS(input.terrain_coords);

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = input.positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.diffuse_color = surface_color;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, 1.0f);
}