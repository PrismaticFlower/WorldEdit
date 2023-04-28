#pragma once

#include "bindings.hlsli"

struct sky_mesh_constants {
   float4x4 rotation;

   float movement_scale;
   float offset;
   uint alpha_cutout;
   uint padding;
};

ConstantBuffer<sky_mesh_constants> cb_mesh_constants : register(SKY_MESH_CB_REGISTER);
