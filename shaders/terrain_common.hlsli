#pragma once

#include "bindings.hlsli"
#include "resource_heaps.hlsli"
#include "samplers.hlsli"

#define TERRAIN_MAX_TEXTURES 16

struct terrain_constants_ {
   float2 half_world_size;
   float grid_size;
   float height_scale;

   uint terrain_max_index;
   float inv_terrain_length;

   uint foliage_map_index;
   float inv_grid_size;

   float3 grid_line_color;
   float grid_line_width;
   
   uint4 diffuse_maps_index[TERRAIN_MAX_TEXTURES / 4];

   float3 texture_transform_x[TERRAIN_MAX_TEXTURES];
   float3 texture_transform_y[TERRAIN_MAX_TEXTURES];
   uint pad0;

   bool has_detail_map;
   uint detail_map_index;
   bool has_normal_map;
   uint normal_map_index;

   float normal_map_scale;
   uint3 pad1;

   float3 foliage_color_0;
   float foliage_transparency;
   float3 foliage_color_1;
   float3 foliage_color_2;
   float3 foliage_color_3;
};

struct vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float  fog : FOG;
   float4 weights[TERRAIN_MAX_TEXTURES / 4] : TEXTUREWEIGHTS;
   float3 static_light : LIGHT;

   float4 positionPS : SV_Position;
};

ConstantBuffer<terrain_constants_> terrain_constants : register(TERRAIN_CB_REGISTER);

static Texture2D<uint4> foliage_map = Texture2D_uint_Heap[terrain_constants.foliage_map_index];

float2 get_terrain_texcoords(uint i, float3 positionWS)
{
   return float2(dot(positionWS, terrain_constants.texture_transform_x[i]),
                 dot(positionWS, terrain_constants.texture_transform_y[i]));
}
