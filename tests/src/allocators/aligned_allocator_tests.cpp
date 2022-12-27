#include "pch.h"

#include "allocators/aligned_allocator.hpp"

using namespace std::literals;

namespace we::assets::tests {

namespace {

template<std::size_t alignment>
bool test_aligned_allocator() noexcept
{
   aligned_allocator<float, alignment> alloc;

   float* const memory = alloc.allocate(1);

   const std::uintptr_t uint_ptr = reinterpret_cast<std::uintptr_t>(memory);

   const bool aligned = (uint_ptr % alignment) == 0;

   alloc.deallocate(memory, 1);

   return aligned;
}

}

TEST_CASE("aligned_allocator tests", "[Allocators][AlignedAllocator]")
{
   CHECK(test_aligned_allocator<8>());
   CHECK(test_aligned_allocator<16>());
   CHECK(test_aligned_allocator<32>());
   CHECK(test_aligned_allocator<64>());
   CHECK(test_aligned_allocator<128>());
   CHECK(test_aligned_allocator<256>());
}

}
