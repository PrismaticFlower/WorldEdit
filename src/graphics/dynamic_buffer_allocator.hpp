#pragma once

#include "gpu/rhi.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <shared_mutex>
#include <vector>

namespace we::graphics {

struct dynamic_buffer_allocator {
   constexpr static std::size_t alignment =
      gpu::constant_buffer_data_placement_alignment;
   constexpr static std::size_t max_pages = 256;

   dynamic_buffer_allocator(const uint32 page_size, gpu::device& device);

   ~dynamic_buffer_allocator();

   struct allocation {
      std::byte* cpu_address;
      gpu_virtual_address gpu_address;
      std::size_t size;
      gpu::resource_handle resource;
      std::size_t offset;
   };

   [[nodiscard]] auto allocate(const std::size_t size) -> allocation;

   template<typename T>
   [[nodiscard]] auto allocate_and_copy(const T& value) -> allocation
   {
      static_assert(std::is_trivially_copyable_v<T>);

      allocation allocation = allocate(sizeof(T));

      std::memcpy(allocation.cpu_address, &value, sizeof(T));

      return allocation;
   }

   void reset(const std::size_t new_frame_index);

private:
   struct page {
      std::size_t allocated = 0;
      gpu::resource_handle buffer = gpu::null_resource_handle;
      std::byte* cpu_base_address = nullptr;
      gpu_virtual_address gpu_base_address = 0;
   };

   std::atomic<page*> _page = nullptr;

   std::shared_mutex _page_change_mutex;

   const std::size_t _page_size = 0;
   std::size_t _frame_index = 0;
   std::size_t _page_index = 0;

   std::array<std::vector<page>, gpu::frame_pipeline_length> _frame_pages;

   std::size_t _frame_section_size = 0;
   gpu::device& _device;
};

}
