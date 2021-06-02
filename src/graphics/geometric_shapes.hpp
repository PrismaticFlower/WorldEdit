#pragma once

#include "gpu/buffer.hpp"
#include "gpu/device.hpp"
#include "types.hpp"

#include <d3d12.h>

namespace we::graphics {

struct geometric_shape {
   uint32 index_count;
   D3D12_INDEX_BUFFER_VIEW index_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW position_vertex_buffer_view;
};

class geometric_shapes {
public:
   explicit geometric_shapes(gpu::device& device);

   auto cube() noexcept -> geometric_shape
   {
      return _cube;
   }

   auto cylinder() noexcept -> geometric_shape
   {
      return _cylinder;
   }

   auto icosphere() noexcept -> geometric_shape
   {
      return _icosphere;
   }

   auto octahedron() -> geometric_shape
   {
      return _octahedron;
   }

   auto cone() -> geometric_shape
   {
      return _cone;
   }

private:
   void init_gpu_buffer(gpu::device& device);

   void init_shapes();

   gpu::buffer _gpu_buffer;

   geometric_shape _icosphere;
   geometric_shape _cylinder;
   geometric_shape _cube;
   geometric_shape _octahedron;
   geometric_shape _cone;
};

}
