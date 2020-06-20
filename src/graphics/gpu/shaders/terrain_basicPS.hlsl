
struct terrain_constants_ {
   float2 half_world_size;
   float grid_size;
   float height_scale;
};

ConstantBuffer<terrain_constants_> terrain_constants : register(b1);

Texture2D<float> height_map : register(t0);

SamplerState bilinear_sampler : register(s0);

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float2 terrain_coords : TERRAINCOORDS;
};

float4 main(input_vertex input) : SV_Target0
{
   const float3 light_normalWS =
      normalize(float3(-159.264923, 300.331013, -66.727310));

   const float height_scale = terrain_constants.height_scale;
   const float grid_size = terrain_constants.grid_size;

   const float height0x =
      height_map.Sample(bilinear_sampler, input.terrain_coords, int2(-1, 0));
   const float height1x =
      height_map.Sample(bilinear_sampler, input.terrain_coords, int2(1, 0));
   const float height0z =
      height_map.Sample(bilinear_sampler, input.terrain_coords, int2(0, -1));
   const float height1z =
      height_map.Sample(bilinear_sampler, input.terrain_coords, int2(0, 1));

   const float3 normalWS =
      normalize(float3((height0x - height1x) * height_scale / (grid_size * 2.0), 1.0,
                       (height0z - height1z) * height_scale / (grid_size * 2.0)));

   float3 light = saturate(dot(normalWS, light_normalWS));

   return float4(light * 0.6 + (1.0 - light) * 0.05.xxx, 1.0f);
}