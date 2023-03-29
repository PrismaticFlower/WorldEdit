#pragma once

#include "types.hpp"

#include "copy_command_list_pool.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"

namespace we::graphics {

struct geometric_shape {
   uint32 index_count;
   gpu::index_buffer_view index_buffer_view;
   gpu::vertex_buffer_view position_vertex_buffer_view;
};

class geometric_shapes {
public:
   explicit geometric_shapes(gpu::device& device,
                             copy_command_list_pool& copy_command_list_pool);

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

   auto hint_hexahedron() -> geometric_shape
   {
      return _hint_hexahedron;
   }

   auto cone() -> geometric_shape
   {
      return _cone;
   }

private:
   void init_gpu_buffer(gpu::device& device,
                        copy_command_list_pool& copy_command_list_pool);

   void init_shapes(gpu::device& device);

   gpu::unique_resource_handle _gpu_buffer;

   geometric_shape _icosphere;
   geometric_shape _cylinder;
   geometric_shape _cube;
   geometric_shape _octahedron;
   geometric_shape _hint_hexahedron;
   geometric_shape _cone;
};

}
