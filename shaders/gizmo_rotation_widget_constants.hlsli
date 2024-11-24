#pragma once

#include "bindings.hlsli"

struct gizmo_rotation_widget_constants {
   float3 bbox_positionVS;
   float inner_radius_sq;
   float3 bbox_scaleVS;
   float outer_radius_sq;

   float3 positionVS;

   float3 x_axisVS;
   bool x_visible;

   float3 y_axisVS;
   bool y_visible;

   float3 z_axisVS;
   bool z_visible;

   float3 x_color;
   float3 y_color;
   float3 z_color;
};

ConstantBuffer<gizmo_rotation_widget_constants> cb_gizmo : register(GIZMO_SHAPE_CB_REGISTER);
