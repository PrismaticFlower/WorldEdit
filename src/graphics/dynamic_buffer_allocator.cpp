
#include "dynamic_buffer_allocator.hpp"
#include "gpu/exception.hpp"
#include "math/align.hpp"

#include <bit>

namespace we::graphics {

dynamic_buffer_allocator::dynamic_buffer_allocator(const uint32 start_page_size,
                                                   gpu::device& device)
   : _page_size{math::align_up(start_page_size, alignment)}, _device{device}
{
   try {
      for (std::vector<page>& pages : _frame_pages) {
         pages.resize(max_pages);

         page new_page{
            .buffer = device.create_buffer({.size = _page_size, .debug_name = "Dynamic Buffer Allocator Page"},
                                           gpu::heap_type::upload)};

         new_page.cpu_base_address =
            static_cast<std::byte*>(device.map(new_page.buffer, 0, {0, 0}));
         new_page.gpu_base_address = device.get_gpu_virtual_address(new_page.buffer);

         pages[0] = new_page;
      }

      _page = &_frame_pages[0][0];
   }
   catch (...) {
      for (std::vector<page>& pages : _frame_pages) {
         for (page& page : pages) {
            if (page.buffer != gpu::null_resource_handle) {
               _device.unsynced_release_resource(page.buffer);
            }
         }
      }

      throw;
   }
}

dynamic_buffer_allocator::~dynamic_buffer_allocator()
{
   for (auto& pages : _frame_pages) {
      for (auto& page : pages) {
         if (page.buffer != gpu::null_resource_handle) {
            _device.release_resource(page.buffer);
         }
      }
   }
}

[[nodiscard]] auto dynamic_buffer_allocator::allocate(const std::size_t size) -> allocation
{
retry_allocate:
   const std::size_t aligned_size = math::align_up(size, alignment);

   page& page = *_page.load(std::memory_order_acquire);

   const std::size_t allocation_offset =
      std::atomic_ref{page.allocated}.fetch_add(aligned_size, std::memory_order_relaxed);

   [[unlikely]] if (allocation_offset + size > _page_size) {
      [[unlikely]] if (size > _page_size) {
         std::scoped_lock lock{_page_change_mutex};

         if (&page != _page.load(std::memory_order_relaxed)) {
            goto retry_allocate;
         }

         const std::size_t new_page_size = std::bit_ceil(size);

         std::array<std::vector<dynamic_buffer_allocator::page>, gpu::frame_pipeline_length> new_frame_pages;

         try {
            for (std::vector<dynamic_buffer_allocator::page>& pages : new_frame_pages) {
               pages.resize(max_pages);

               dynamic_buffer_allocator::page new_page{
                  .buffer =
                     _device.create_buffer({.size = new_page_size,
                                            .debug_name =
                                               "Dynamic Buffer Allocator Page"},
                                           gpu::heap_type::upload)};

               new_page.cpu_base_address =
                  static_cast<std::byte*>(_device.map(new_page.buffer, 0, {0, 0}));
               new_page.gpu_base_address =
                  _device.get_gpu_virtual_address(new_page.buffer);

               pages[0] = new_page;
            }
         }
         catch (...) {
            for (std::vector<dynamic_buffer_allocator::page>& pages : new_frame_pages) {
               for (dynamic_buffer_allocator::page& release_page : pages) {
                  if (release_page.buffer != gpu::null_resource_handle) {
                     _device.unsynced_release_resource(release_page.buffer);
                  }
               }
            }

            throw gpu::exception{
               gpu::error::out_of_memory,
               "Failed to extend dynamic buffer allocator page size."};
         }

         _deferred_release_frame_pages.push_back(std::move(_frame_pages));
         _frame_pages.swap(new_frame_pages);
         _page_size = new_page_size;

         _page = &_frame_pages[0][0];
      }
      else {
         std::scoped_lock lock{_page_change_mutex};

         if (&page != _page.load(std::memory_order_relaxed)) {
            goto retry_allocate;
         }

         _page_index += 1;

         if (_page_index >= max_pages) {
            throw gpu::exception{gpu::error::out_of_memory,
                                 "Dynamic buffer allocator is out of pages."};
         }

         auto& new_page = _frame_pages[_frame_index][_page_index];

         [[unlikely]] if (new_page.buffer == gpu::null_resource_handle) {
            new_page.buffer =
               _device.create_buffer({.size = _page_size, .debug_name = "Dynamic Buffer Allocator Page"},
                                     gpu::heap_type::upload);
            new_page.cpu_base_address =
               static_cast<std::byte*>(_device.map(new_page.buffer, 0, {0, 0}));
            new_page.gpu_base_address =
               _device.get_gpu_virtual_address(new_page.buffer);
         }

         _page.store(&new_page);
      }

      goto retry_allocate;
   }

   return {.cpu_address = page.cpu_base_address + allocation_offset,
           .gpu_address = page.gpu_base_address + allocation_offset,
           .size = size,
           .resource = page.buffer,
           .offset = allocation_offset};
}

void dynamic_buffer_allocator::reset(const std::size_t new_frame_index)
{
   _frame_index = new_frame_index;
   _page_index = 0;
   _page = &_frame_pages[_frame_index][0];

   for (page& page : _frame_pages[_frame_index]) page.allocated = 0;

   for (std::array<std::vector<page>, gpu::frame_pipeline_length>& frame_pages :
        _deferred_release_frame_pages) {
      for (std::vector<page>& pages : frame_pages) {
         for (page& page : pages) {
            if (page.buffer != gpu::null_resource_handle) {
               _device.release_resource(page.buffer);
            }
         }
      }
   }

   _deferred_release_frame_pages.clear();
}
}
