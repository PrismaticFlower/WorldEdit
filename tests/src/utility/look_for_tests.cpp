#include "pch.h"

#include "utility/look_for.hpp"

#include <array>

namespace we::tests {

TEST_CASE("look_for tests", "[Utility][Find]")
{
   std::array values{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

   REQUIRE(look_for(values, 0) == &values[0]);
   REQUIRE(look_for(values, 1) == &values[1]);
   REQUIRE(look_for(values, 100) == nullptr);

   REQUIRE(look_for(values, 0ll) == &values[0]);
   REQUIRE(look_for(values, 1ll) == &values[1]);
   REQUIRE(look_for(values, 100ll) == nullptr);
}

TEST_CASE("look_for predicate tests", "[Utility][Find]")
{
   std::array values{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

   REQUIRE(look_for(values, [](int i) { return 0 == i; }) == &values[0]);
   REQUIRE(look_for(values, [](int i) { return 1 == i; }) == &values[1]);
   REQUIRE(look_for(values, [](int) { return false; }) == nullptr);
}

}
