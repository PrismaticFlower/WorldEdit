
#include "dynamic_buffer_allocator.hpp"
#include "device.hpp"

namespace we::graphics::gpu {

dynamic_buffer_allocator::dynamic_buffer_allocator(const UINT max_size, device& device)
   : _buffer_size{max_size}
{
   for (auto& backing_buffer : _buffers) {
      backing_buffer.underlying_buffer =
         device.create_buffer({.size = max_size}, D3D12_HEAP_TYPE_UPLOAD,
                              D3D12_RESOURCE_STATE_GENERIC_READ);

      const D3D12_RANGE read_range{};
      void* data = nullptr;
      throw_if_failed(
         backing_buffer.underlying_buffer.resource()->Map(0, &read_range, &data));

      backing_buffer.cpu_address = static_cast<std::byte*>(data);
      backing_buffer.gpu_address =
         backing_buffer.underlying_buffer.gpu_virtual_address();
   }

   reset(device.frame_index);
}

}
