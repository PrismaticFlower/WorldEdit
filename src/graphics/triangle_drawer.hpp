#pragma once

#include "gpu/command_list.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "types.hpp"

namespace we::graphics {

/// @brief Helper for drawing and batching dynamic triangles. All draw state except vertex buffers should be setup before use.
class triangle_drawer {
public:
   /// @brief Construct a triangle_drawer.
   /// @param command_list The command_list for placing draw calls onto.
   /// @param buffer_allocator The allocator to use for allocating space for the triangles.
   /// @param max_buffered_triangles The max number of triangles that can be buffered before automatically calling submit.
   triangle_drawer(gpu::graphics_command_list& command_list,
                   gpu::dynamic_buffer_allocator& buffer_allocator,
                   const uint32 max_buffered_triangles);

   /// @brief Add a triangle.
   /// @param v0
   /// @param v1
   /// @param v2
   void add(const float3 v0, const float3 v1, const float3 v2);

   /// @brief Submit all buffered triangles, this may be called by add if the buffer fills up. It should always be called after you are finished calling add.
   void submit();

private:
   gpu::graphics_command_list& _command_list;
   gpu::dynamic_buffer_allocator& _buffer_allocator;
   const uint32 _max_buffered_triangles = 0;

   gpu::dynamic_buffer_allocator::allocation _current_allocation;
   uint32 _batch_vertices = 0;
   uint32 _batch_offset = 0;
   uint32 _buffered_triangles = 0;
};

}
