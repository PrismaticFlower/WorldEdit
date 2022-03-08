#include "pch.h"

#include "utility/overload.hpp"

namespace we::tests {

TEST_CASE("overload tests", "[Utility][Overload]")
{
   overload set{[](float) { return 0; }, [](int) { return 1; },
                [](char) { return 2; }};

   REQUIRE(set(1.0f) == 0);
   REQUIRE(set(1) == 1);
   REQUIRE(set('1') == 2);
}

}
