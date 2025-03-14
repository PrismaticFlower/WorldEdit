#pragma once

#include "types.hpp"

namespace we::graphics {

struct alignas(256) frame_constant_buffer {
   float4x4 projection_from_world;
   float4x4 projection_from_view;

   float3 view_positionWS;
   float texture_scroll_duration;

   float2 viewport_size;
   float2 viewport_topleft;

   float line_width;
};

static_assert(sizeof(frame_constant_buffer) == 256);

struct alignas(256) wireframe_constant_buffer {
   float3 color;
};

static_assert(sizeof(wireframe_constant_buffer) == 256);

struct alignas(256) meta_outlined_constant_buffer {
   float4 color;
   float4 outline_color;
};

static_assert(sizeof(meta_outlined_constant_buffer) == 256);

}