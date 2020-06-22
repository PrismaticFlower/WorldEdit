
#include "terrain_common.hlsli"

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float2 terrain_coords : TERRAINCOORDS;
};

float4 main(input_vertex input) : SV_Target0
{
   const float3 light_normalWS =
      normalize(float3(-159.264923, 300.331013, -66.727310));

   const float3 normalWS = get_terrain_normalWS(input.terrain_coords);

   float3 light = saturate(dot(normalWS, light_normalWS));

   return float4((light * 0.4 + (1.0 - light) * 0.033).xxx, 1.0f);
}