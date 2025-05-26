#include "offset_allocator.hpp"

#include <offsetAllocator.hpp>

namespace we {

struct offset_allocator::impl {
   impl(uint32 size, uint32 max_allocations)
      : _allocator{size, max_allocations + 1}
   // + 1 for https://github.com/sebbbi/OffsetAllocator/issues/3
   {
   }

   auto allocate(uint32 size) noexcept -> allocation
   {
      static_assert(OffsetAllocator::Allocation::NO_SPACE ==
                    offset_allocator::allocation::no_space);

      const OffsetAllocator::Allocation allocation = _allocator.allocate(size);

      return {.offset = allocation.offset, .metadata = allocation.metadata};
   }

   void free(allocation allocation) noexcept
   {
      _allocator.free({.offset = allocation.offset, .metadata = allocation.metadata});
   }

private:
   OffsetAllocator::Allocator _allocator;
};

offset_allocator::offset_allocator(uint32 size, uint32 max_allocations)
   : _impl{std::make_unique<impl>(size, max_allocations)}
{
}

offset_allocator::~offset_allocator() = default;

auto offset_allocator::allocate(uint32 size) noexcept -> allocation
{
   return _impl->allocate(size);
}

void offset_allocator::free(allocation allocation) noexcept
{
   return _impl->free(allocation);
}

offset_allocator::allocation::operator bool() const noexcept
{
   return offset != no_space;
}

offset_allocator_aligned::offset_allocator_aligned(uint32 size, uint32 alignment,
                                                   uint32 max_allocations)
   : _allocator{size / alignment, max_allocations}, _alignment{alignment}
{
}

auto offset_allocator_aligned::allocate(uint32 size) noexcept -> allocation
{
   const uint32 block_count = (size + _alignment - 1u) / _alignment;

   const allocation block_allocation = _allocator.allocate(block_count);

   if (not block_allocation) return {};

   return {.offset = block_allocation.offset * _alignment,
           .metadata = block_allocation.metadata};
}

void offset_allocator_aligned::free(allocation allocation) noexcept
{
   _allocator.free({.offset = allocation.offset / _alignment,
                    .metadata = allocation.metadata});
}

}