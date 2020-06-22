
struct terrain_constants_ {
   float2 half_world_size;
   float grid_size;
   float height_scale;
};

struct patch_constants_ {
   uint x;
   uint y;
};

ConstantBuffer<terrain_constants_> terrain_constants : register(b0, space2);
Texture2D<float> height_map : register(t0, space2);
StructuredBuffer<patch_constants_> patch_constants : register(t1, space2);

SamplerState bilinear_sampler : register(s0);

const static uint patch_point_count = 17;

float3 get_terrain_normalWS(float2 terrain_coords)
{
   const float height_scale = terrain_constants.height_scale;
   const float grid_size = terrain_constants.grid_size;

   const float height0x =
      height_map.Sample(bilinear_sampler, terrain_coords, int2(-1, 0));
   const float height1x =
      height_map.Sample(bilinear_sampler, terrain_coords, int2(1, 0));
   const float height0z =
      height_map.Sample(bilinear_sampler, terrain_coords, int2(0, -1));
   const float height1z =
      height_map.Sample(bilinear_sampler, terrain_coords, int2(0, 1));

   const float3 normalWS =
      normalize(float3((height0x - height1x) * height_scale / (grid_size * 2.0), 1.0,
                       (height0z - height1z) * height_scale / (grid_size * 2.0)));

   return normalWS;
}
