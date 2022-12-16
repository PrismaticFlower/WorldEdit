#pragma once

#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "math/align.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstring>

namespace we::graphics {

struct dynamic_buffer_allocator {
   constexpr static std::size_t alignment =
      gpu::constant_buffer_data_placement_alignment;

   explicit dynamic_buffer_allocator(const uint32 max_size, gpu::device& device);

   struct allocation {
      std::byte* cpu_address;
      gpu_virtual_address gpu_address;
      std::size_t size;
      std::size_t offset;
   };

   [[nodiscard]] auto allocate(const std::size_t size) -> allocation
   {
      const std::size_t aligned_size = math::align_up(size, alignment);
      const std::size_t allocation_offset =
         _buffer_base_offset +
         _allocated.fetch_add(aligned_size, std::memory_order_relaxed);

      [[unlikely]] if ((allocation_offset + aligned_size - _buffer_base_offset) >
                       _frame_section_size) {
         std::terminate(); // OOM, let's go and cry now.
      }

      return {.cpu_address = _cpu_base_address + allocation_offset,
              .gpu_address = _gpu_base_address + allocation_offset,
              .size = size,
              .offset = allocation_offset};
   }

   template<typename T>
   [[nodiscard]] auto allocate_and_copy(const T& value) -> allocation
   {
      static_assert(std::is_trivially_copyable_v<T>);

      allocation allocation = allocate(sizeof(T));

      std::memcpy(allocation.cpu_address, &value, sizeof(T));

      return allocation;
   }

   auto resource() -> gpu::resource_handle
   {
      return _buffer.get();
   }

   void reset(const std::size_t new_frame_index)
   {
      _allocated = 0;
      _buffer_base_offset = (_frame_section_size * new_frame_index);
   }

private:
   std::atomic_size_t _allocated = 0;
   std::size_t _buffer_base_offset = 0;

   gpu::unique_resource_handle _buffer;
   std::byte* _cpu_base_address = nullptr;
   gpu_virtual_address _gpu_base_address = 0;

   std::size_t _frame_section_size = 0;
};

}
