#include "pch.h"

#include "types.hpp"
#include "utility/make_from_bytes.hpp"

#include <algorithm>

using namespace Catch::literals;

namespace sk::utility::tests {

TEST_CASE("make from bytes", "[Utility][MakeFromBytes]")
{
   REQUIRE(make_from_bytes<float>(std::array{std::byte{0xdb}, std::byte{0x0f},
                                             std::byte{0x49}, std::byte{0x40}}) ==
           3.141592_a);

   REQUIRE(make_from_bytes<int32>(std::array{std::byte{0xff}, std::byte{0x00},
                                             std::byte{0x00}, std::byte{0xff}}) ==
           -16776961);
}

}