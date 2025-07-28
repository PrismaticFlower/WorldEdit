#pragma once

#include "bindings.hlsli"

#define MAX_BLOCK_MATERIALS 256

enum block_material_flags {
   block_material_none = 0b0,
   block_material_none_has_gloss = 0b1,
   block_material_has_normal_map = 0b10,
   block_material_has_detail_map = 0b100,
   block_material_has_env_map = 0b1000,
   block_material_tile_normal_map = 0b10000,
};

struct block_material {
   block_material_flags flags;
   uint diffuse_map_index;
   uint normal_map_index;
   uint detail_map_index;
   float3 specular_color;
   uint env_map_index;
   float2 detail_scale;
   uint2 padding0;
   float3 env_color;
   uint padding1;
};

cbuffer BlockMaterials : register(BLOCK_MATERIALS_CB_REGISTER)
{
   block_material block_materials[MAX_BLOCK_MATERIALS];
}

