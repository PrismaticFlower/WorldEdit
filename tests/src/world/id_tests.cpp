#include "pch.h"

#include "world/id.hpp"

#include <bit>
#include <concepts>
#include <type_traits>

namespace we::world::tests {

namespace {

auto make_id_for_test(std::underlying_type_t<id<void>> i) -> id<void>
{
   return std::bit_cast<id<void>>(i);
}

}

TEST_CASE("world id", "[World][ID]")
{
   static_assert(not std::is_same_v<id<int>, id<float>>);
   static_assert(not std::equality_comparable_with<id<int>, id<float>>);

   id_generator<void> next_id;

   id<void> id0 = next_id.aquire();
   id<void> id1 = next_id.aquire();
   id<void> id2 = next_id.aquire();

   // All returned IDs are unique.
   REQUIRE(id0 != id1);
   REQUIRE(id0 != id2);
   REQUIRE(id1 != id2);

   // IDs start at 0 and increase from there.
   REQUIRE(id0 == make_id_for_test(0));
   REQUIRE(id1 == make_id_for_test(1));
   REQUIRE(id2 == make_id_for_test(2));
}

}
