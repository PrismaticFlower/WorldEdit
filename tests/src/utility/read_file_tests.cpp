#include "pch.h"

#include "utility/read_file.hpp"

#include <string_view>

using namespace std::literals;

namespace sk::utility::tests {

TEST_CASE("read file to bytes", "[Utility][ReadFile]")
{
   auto bytes = read_file_to_bytes("data/test.bytes");

   const std::vector<std::byte> expected_bytes{std::byte{0xff},
                                               std::byte{0x65},
                                               std::byte{0x86},
                                               std::byte{0xfb},
                                               std::byte{0xc0},
                                               std::byte{0x20},
                                               std::byte{0x40},
                                               std::byte{0xdf}};

   REQUIRE(bytes == expected_bytes);
   REQUIRE_THROWS(
      read_file_to_bytes("data/some/path/that/does/not/exist/bad.txt"));
}

TEST_CASE("read file to string", "[Utility][ReadFile]")
{

   auto str = read_file_to_string("data/test.txt");

   REQUIRE(str == "Test String"sv);
   REQUIRE_THROWS(
      read_file_to_bytes("data/some/path/that/does/not/exist/bad.txt"));
}

}
