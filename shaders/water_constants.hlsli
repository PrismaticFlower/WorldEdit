#pragma once

#include "bindings.hlsli"

struct water_constant_buffer
{
   float2 tiling;
   float2 velocity;
   float height;
   uint color_map_index;
   float4 color;
};

ConstantBuffer<water_constant_buffer> cb_water : register(WATER_CB_REGISTER);
