
#include "triangle_drawer.hpp"

namespace we::graphics {

triangle_drawer::triangle_drawer(gpu::graphics_command_list& command_list,
                                 gpu::dynamic_buffer_allocator& buffer_allocator,
                                 const uint32 max_buffered_triangles)
   : _command_list{command_list},
     _buffer_allocator{buffer_allocator},
     _max_buffered_triangles{max_buffered_triangles},
     _current_allocation{_buffer_allocator.allocate(
        _max_buffered_triangles * sizeof(std::array<float3, 3>))}
{
}

void triangle_drawer::add(const float3 v0, const float3 v1, const float3 v2)
{
   std::array<float3, 3> tri{v0, v1, v2};

   std::memcpy(_current_allocation.cpu_address +
                  (sizeof(std::array<float3, 3>) * _buffered_triangles),
               &tri, sizeof(std::array<float3, 3>));

   _buffered_triangles += 1;
   _batch_vertices += 3;

   if (_buffered_triangles == _max_buffered_triangles) {
      submit();

      _current_allocation = _buffer_allocator.allocate(
         _max_buffered_triangles * sizeof(std::array<float3, 3>));
      _buffered_triangles = 0;
      _batch_offset = 0;
   }
}

void triangle_drawer::submit()
{
   if (_batch_vertices == 0) return;

   const D3D12_VERTEX_BUFFER_VIEW vbv{.BufferLocation = _current_allocation.gpu_address,
                                      .SizeInBytes =
                                         static_cast<UINT>(_current_allocation.size),
                                      .StrideInBytes = sizeof(float3)};

   _command_list.ia_set_vertex_buffers(0, vbv);
   _command_list.draw_instanced(_batch_vertices, 1, _batch_offset, 0);

   _batch_offset += _batch_vertices;
   _batch_vertices = 0;
}

}
