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
         return static_cast<uint32>(new_allocation);
      }
   }

   /// @brief Free a descriptor allowing it to be reused.
   /// @param index The index of the descriptor.
   void free(const uint32 index)
   {
      std::scoped_lock lock{_mutex};

      _free_descriptors.push_back(index);
   }

private:
   std::atomic_uint64_t _next_descriptor = 0;
   const uint64 _heap_size = 0;

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
      if (_allocator) _allocator->free(_index);
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
