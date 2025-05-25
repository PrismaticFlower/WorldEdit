#pragma once

#include "rhi.hpp"

namespace we::graphics::gpu {

struct indirect_draw_arguments {
   uint32 vertex_count_per_instance;
   uint32 instance_count;
   uint32 start_vertex_location;
   uint32 start_instance_location;
};

static_assert(sizeof(indirect_draw_arguments) == 16);

struct indirect_draw_indexed_arguments {
   uint32 index_count_per_instance;
   uint32 instance_count;
   uint32 start_index_location;
   int32 base_vertex_location;
   uint32 start_instance_location;
};

static_assert(sizeof(indirect_draw_indexed_arguments) == 20);

struct indirect_dispatch_arguments {
   uint32 thread_group_count_x;
   uint32 thread_group_count_y;
   uint32 thread_group_count_z;
};

static_assert(sizeof(indirect_dispatch_arguments) == 12);

using indirect_dispatch_mesh_arguments = indirect_dispatch_arguments;

using indirect_vertex_buffer_view = vertex_buffer_view;

static_assert(sizeof(indirect_vertex_buffer_view) == 16);

struct indirect_index_buffer_view {
   gpu_virtual_address buffer_location;
   uint32 size_in_bytes;
   DXGI_FORMAT format;
};

static_assert(sizeof(indirect_index_buffer_view) == 16);

}