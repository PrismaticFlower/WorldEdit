#pragma once

#include "bindings.hlsli"

struct gizmo_line_constants {
   float3 startVS;
   float3 endVS;

   float3 extensionVS;

   float3 color;
};

ConstantBuffer<gizmo_line_constants> cb_gizmo_line : register(GIZMO_SHAPE_CB_REGISTER);
