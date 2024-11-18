#pragma once

#include "bindings.hlsli"

struct gizmo_cone_constants {
   float3 bbox_positionVS;
   float3 bbox_scaleVS;

   float3 startVS;
   float base_radius;
   float3 endVS;

   float3 color;
};

ConstantBuffer<gizmo_cone_constants> cb_gizmo_cone : register(GIZMO_CONE_CB_REGISTER);
