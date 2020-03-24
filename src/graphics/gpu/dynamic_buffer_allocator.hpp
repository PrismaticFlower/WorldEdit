#pragma once

#include "buffer.hpp"
#include "common.hpp"
#include "device.hpp"
#include "hresult_error.hpp"
#include "math/align.hpp"

#include <array>
#include <atomic>
#include <cstddef>

#include <d3d12.h>

namespace sk::graphics::gpu {

class dynamic_buffer_allocator {
public:
   constexpr static std::size_t alignment =
      D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

   explicit dynamic_buffer_allocator(const UINT max_size, device& device)
      : _buffer_size{max_size}
   {
      for (auto& backing_buffer : _buffers) {
         backing_buffer.underlying_buffer =
            buffer{device, max_size, D3D12_HEAP_TYPE_UPLOAD,
                   D3D12_RESOURCE_STATE_GENERIC_READ};

         const D3D12_RANGE read_range{};
         void* data = nullptr;
         throw_if_failed(
            backing_buffer.underlying_buffer.resource->Map(0, &read_range, &data));

         backing_buffer.cpu_address = static_cast<std::byte*>(data);
         backing_buffer.gpu_address =
            backing_buffer.underlying_buffer.resource->GetGPUVirtualAddress();
      }

      reset(device.frame_index);
   }

   struct allocation {
      std::byte* cpu_address;
      D3D12_GPU_VIRTUAL_ADDRESS gpu_address;
      std::size_t size;
   };

   [[nodiscard]] auto allocate(const std::size_t size) -> allocation
   {
      const std::size_t aligned_size = math::align_up(size, alignment);
      const std::size_t allocation_offset =
         _allocated.fetch_add(aligned_size, std::memory_order_relaxed);

      [[unlikely]] if ((allocation_offset + aligned_size) > _buffer_size)
      {
         throw hresult_exception{E_OUTOFMEMORY};
      }

      return {.cpu_address = _cpu_base_address + allocation_offset,
              .gpu_address = _gpu_base_address + allocation_offset,
              .size = size};
   }

   void reset(const std::size_t new_frame_index)
   {
      _allocated = 0;
      _cpu_base_address = _buffers[new_frame_index].cpu_address;
      _gpu_base_address = _buffers[new_frame_index].gpu_address;
   }

private:
   std::atomic_size_t _allocated = 0;
   std::size_t _buffer_size = 0;
   std::byte* _cpu_base_address = nullptr;
   D3D12_GPU_VIRTUAL_ADDRESS _gpu_base_address = 0;

   struct backing_buffer {
      buffer underlying_buffer;
      std::byte* cpu_address = nullptr;
      D3D12_GPU_VIRTUAL_ADDRESS gpu_address = 0;
   };

   std::array<backing_buffer, render_latency> _buffers;
};

}
