#pragma once

#include "bindings.hlsli"
#include "samplers.hlsli"

#define TERRAIN_MAX_TEXTURES 16

struct terrain_constants_ {
   float2 half_world_size;
   float grid_size;
   float height_scale;

   uint height_map_index;
   uint texture_weight_maps_index;
   uint color_map_index;

   uint diffuse_maps_index[TERRAIN_MAX_TEXTURES];

   float3 texture_transform_x[TERRAIN_MAX_TEXTURES];
   float3 texture_transform_y[TERRAIN_MAX_TEXTURES];
};

struct patch_info {
   uint x;
   uint y;
   uint active_textures;
   uint padding;
};

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float2 terrain_coords : TERRAINCOORDS;
   nointerpolation uint active_textures : ACTIVETEXTURES;
   float3 color : COLOR;

   float4 positionSS : SV_Position;
};

ConstantBuffer<terrain_constants_> terrain_constants : register(TERRAIN_CB_REGISTER);
StructuredBuffer<patch_info> patch_constants : register(TERRAIN_PATCH_DATA_REGISTER);

static Texture2D<float> height_map = ResourceDescriptorHeap[terrain_constants.height_map_index];
static Texture2DArray<float> texture_weight_maps =
   ResourceDescriptorHeap[terrain_constants.texture_weight_maps_index];
static Texture2D<float4> color_map = ResourceDescriptorHeap[terrain_constants.color_map_index];

const static uint patch_point_count = 17;
const static uint terrain_max_textures = TERRAIN_MAX_TEXTURES;

float3 get_terrain_normalWS(float2 terrain_coords)
{
   const float height_scale = terrain_constants.height_scale;
   const float grid_size = terrain_constants.grid_size;

   const float height0x = height_map.Sample(sampler_bilinear_clamp, terrain_coords, int2(-1, 0));
   const float height1x = height_map.Sample(sampler_bilinear_clamp, terrain_coords, int2(1, 0));
   const float height0z = height_map.Sample(sampler_bilinear_clamp, terrain_coords, int2(0, -1));
   const float height1z = height_map.Sample(sampler_bilinear_clamp, terrain_coords, int2(0, 1));

   const float3 normalWS = normalize(float3((height0x - height1x) * height_scale / (grid_size * 2.0), 1.0,
                                            (height0z - height1z) * height_scale / (grid_size * 2.0)));

   return normalWS;
}

float2 get_terrain_texcoords(uint i, float3 positionWS)
{
   return float2(dot(positionWS, terrain_constants.texture_transform_x[i]),
                 dot(positionWS, terrain_constants.texture_transform_y[i]));
}
