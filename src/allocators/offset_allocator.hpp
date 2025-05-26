#pragma once

#include "types.hpp"

#include <memory>

namespace we {

struct offset_allocator {
   struct allocation {
      static constexpr uint32 no_space = 0xff'ff'ff'ffu;

      uint32 offset = no_space;
      uint32 metadata = no_space;

      operator bool() const noexcept;
   };

   offset_allocator(uint32 size, uint32 max_allocations);

   ~offset_allocator();

   offset_allocator(const offset_allocator&) noexcept = delete;
   auto operator=(const offset_allocator&) noexcept -> offset_allocator& = delete;

   offset_allocator(offset_allocator&&) noexcept = delete;
   auto operator=(offset_allocator&&) noexcept -> offset_allocator& = delete;

   auto allocate(uint32 size) noexcept -> allocation;

   void free(allocation allocation) noexcept;

private:
   struct impl;

   std::unique_ptr<impl> _impl;
};

struct offset_allocator_aligned {
   using allocation = offset_allocator::allocation;

   offset_allocator_aligned(uint32 size, uint32 alignment, uint32 max_allocations);

   offset_allocator_aligned(const offset_allocator_aligned&) noexcept = delete;
   auto operator=(const offset_allocator_aligned&) noexcept
      -> offset_allocator_aligned& = delete;

   offset_allocator_aligned(offset_allocator_aligned&&) noexcept = delete;
   auto operator=(offset_allocator_aligned&&) noexcept
      -> offset_allocator_aligned& = delete;

   auto allocate(uint32 size) noexcept -> allocation;

   void free(allocation allocation) noexcept;

private:
   struct impl;

   offset_allocator _allocator;
   const uint32 _alignment = 4;
};

}
