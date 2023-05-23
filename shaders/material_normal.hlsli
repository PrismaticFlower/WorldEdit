#pragma once

#include "bindings.hlsli"

enum flags {
   none = 0b0,
   transparent = 0b1,
   unlit = 0b10,
   specular_visibility_in_diffuse_map = 0b100,
   scrolling = 0b1000,
   static_lighting = 0b10000,
   has_normal_map = 0b100000,
   has_detail_map = 0b1000000,
   has_env_map = 0b10000000,
   tile_normal_map = 0b100000000,
   additive = 0b1000000000,
};

struct material_input {
   uint flags;
   uint diffuse_map_index;
   uint normal_map_index;
   uint detail_map_index;
   float3 specular_color;
   uint env_map_index;
   float2 scrolling_amount;
   float2 detail_scale;
   float3 env_color;
};

ConstantBuffer<material_input> material : register(MATERIAL_CB_REGISTER);