#include "pch.h"

#include "types.hpp"
#include "utility/binary_reader.hpp"

#include <algorithm>
#include <array>

using namespace Catch::literals;

namespace we::utility::tests {

TEST_CASE("binary reader", "[Utility][BinaryReader]")
{
   constexpr std::array bytes{std::byte{0xdb}, std::byte{0x0f},
                              std::byte{0x49}, std::byte{0x40},
                              std::byte{0x01}, std::byte{0x00},
                              std::byte{0x00}, std::byte{0x00},
                              std::byte{0x00}, std::byte{0x00},
                              std::byte{0x00}, std::byte{0x10}};

   binary_reader reader{bytes};

   REQUIRE(reader);
   REQUIRE(reader.read<float>() == 3.141592_a);
   REQUIRE(reader.read<int32>() == 1);
   REQUIRE_NOTHROW(reader.skip(2));
   REQUIRE(reader.read<int8>() == 0);
   REQUIRE(reader.read_bytes(1)[0] == std::byte{0x10});

   REQUIRE(not reader);
   REQUIRE_THROWS_AS(reader.read<int32>() == 42, binary_reader_overflow);
   REQUIRE_THROWS_AS(reader.skip(2), binary_reader_overflow);
}

}
