#pragma once

struct frame_constant_buffer {
   float4x4 view_projection_matrix;

   float2 viewport_size;
   float2 viewport_topleft;

   float line_width;
};
