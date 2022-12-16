#include "pch.h"

#include "graphics/gpu/detail/descriptor_allocator.hpp"

namespace we::graphics::gpu::detail::tests {

TEST_CASE("descriptor_allocator tests", "[Graphics][GPU]")
{
   descriptor_allocator allocator{4};

   // Test allocations.
   REQUIRE(allocator.allocate() == 0);
   REQUIRE(allocator.allocate() == 1);
   REQUIRE(allocator.allocate() == 2);
   REQUIRE(allocator.allocate() == 3);

   // Test OOM failure.
   REQUIRE_THROWS_AS(allocator.allocate(), gpu::exception);

   // Test freeing and reallocating.
   allocator.free(2);

   REQUIRE(allocator.allocate() == 2);
}

}
