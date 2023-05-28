#pragma once

#include "bindings.hlsli"

struct water_constant_buffer
{
   float4 color;
   float height;
   uint color_map_index;
};

ConstantBuffer<water_constant_buffer> cb_water : register(WATER_CB_REGISTER);
