#pragma once

#include "bindings.hlsli"

struct frame_constant_buffer {
   float4x4 world_from_view;
   float4x4 projection_from_world;
   float4x4 projection_from_view;

   float3 view_positionWS;
   float texture_scroll_duration;

   float2 viewport_size;
   float2 viewport_topleft;

   float line_width;
};

ConstantBuffer<frame_constant_buffer> cb_frame : register(FRAME_CB_REGISTER);
