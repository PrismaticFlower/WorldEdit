#pragma once

#include "../exception.hpp"
#include "types.hpp"

#include <atomic>
#include <shared_mutex>
#include <utility>
#include <vector>

namespace we::graphics::gpu::detail {

struct descriptor_allocator {
   /// @brief Make a descriptor allocator to manage heap_size descriptors.
   /// @param heap_size The count of descriptors in the heap being managed.
   explicit descriptor_allocator(uint32 heap_size) : _heap_size{heap_size} {};

   /// @brief Allocate a single descriptor.
   /// @return The index of the descriptor in the heap.
   [[nodiscard]] auto allocate() -> uint32
   {
      // Try to reuse an allocation.
      {
         std::scoped_lock lock{_mutex};

         if (not _free_descriptors.empty()) {
            const uint32 reused_allocation = _free_descriptors.back();

            _free_descriptors.pop_back();
            _descriptor_references[reused_allocation].store(1, std::memory_order_relaxed);

            return reused_allocation;
         }
      }

      const uint64 new_allocation =
         _next_descriptor.fetch_add(1, std::memory_order_relaxed);

      if (new_allocation >= _heap_size) {
         throw exception{error ::out_of_memory,
                         "A descriptor heap ran out of memory."};
      }
      else {
         _descriptor_references[new_allocation].store(1, std::memory_order_relaxed);

         return static_cast<uint32>(new_allocation);
      }
   }

   /// @brief Release a reference to a descriptor.
   /// @param index The index of the descriptor.
   void release(const uint32 index)
   {
      if (index >= _heap_size) std::terminate();

      const uint32 ref_count =
         _descriptor_references[index].fetch_sub(1, std::memory_order_relaxed) - 1;

      // Check for double-free/programming error.
      if (ref_count == UINT32_MAX) std::terminate();

      if (ref_count == 0) {
         std::scoped_lock lock{_mutex};

         _free_descriptors.push_back(index);
      }
   }

   /// @brief Add a reference to an existing descriptor.
   /// @param index The index of the descriptor.
   void add_ref(const uint32 index) noexcept
   {
      if (index >= _heap_size) std::terminate();

      const uint32 ref_count =
         _descriptor_references[index].fetch_add(1, std::memory_order_relaxed) + 1;

      // Check for adding a ref to an unallocated descriptor. This would indicate a programming error or corruption.
      if (ref_count == 1) std::terminate();
   }

private:
   std::atomic_uint64_t _next_descriptor = 0;
   const uint64 _heap_size = 0;

   const std::unique_ptr<std::atomic_uint32_t[]> _descriptor_references =
      std::make_unique<std::atomic_uint32_t[]>(_heap_size);

   std::shared_mutex _mutex;
   std::vector<uint32> _free_descriptors;
};

/// @brief For use with release_queue
struct unique_descriptor_releaser {
   unique_descriptor_releaser() = default;

   unique_descriptor_releaser(uint32 index, descriptor_allocator& allocator)
      : _allocator{&allocator}, _index{index}
   {
   }

   unique_descriptor_releaser(unique_descriptor_releaser&& other) noexcept
   {
      unique_descriptor_releaser discard;

      discard.swap(other);
      this->swap(discard);
   }

   auto operator=(unique_descriptor_releaser&& other) -> unique_descriptor_releaser&
   {
      unique_descriptor_releaser discard;

      discard.swap(other);
      this->swap(discard);

      return *this;
   }

   unique_descriptor_releaser(const unique_descriptor_releaser&) = delete;
   auto operator=(const unique_descriptor_releaser&)
      -> unique_descriptor_releaser& = delete;

   ~unique_descriptor_releaser()
   {
      if (_allocator) _allocator->release(_index);
   }

   void swap(unique_descriptor_releaser& other) noexcept
   {
      using std::swap;

      swap(this->_allocator, other._allocator);
      swap(this->_index, other._index);
   }

private:
   descriptor_allocator* _allocator = nullptr;
   uint32 _index = 0;
};

}
