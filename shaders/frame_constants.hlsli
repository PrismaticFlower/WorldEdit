#pragma once

#include "bindings.hlsli"

struct frame_constant_buffer {
   float4x4 view_projection_matrix;

   float3 view_positionWS;

   float2 viewport_size;
   float2 viewport_topleft;

   float line_width;
};

ConstantBuffer<frame_constant_buffer> cb_frame : register(FRAME_CB_REGISTER);
