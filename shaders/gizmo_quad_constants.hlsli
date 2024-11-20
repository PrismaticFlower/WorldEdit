#pragma once

#include "bindings.hlsli"

struct gizmo_quad_constants {
   float3 corner0WS;
   float3 corner1WS;
   float3 corner2WS;
   float3 corner3WS;

   float outline_width;

   float3 color;
   float inner_alpha;
};

ConstantBuffer<gizmo_quad_constants> cb_gizmo_quad : register(GIZMO_SHAPE_CB_REGISTER);
