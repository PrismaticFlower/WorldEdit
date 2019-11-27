#include "pch.h"

#include <utility/string_ops.hpp>

using namespace std::literals;

namespace sk::utility::string::tests {

TEST_CASE("string count lines", "[Utility][StringOp]")
{
   const auto input = "Line0\nLine1\nLine2\n"sv;

   REQUIRE(count_lines(input) == 3);
}

TEST_CASE("string split first of inclusive", "[Utility][StringOp]")
{
   const auto input = "Foo Bar Baz"sv;

   REQUIRE(split_first_of_inclusive(input, "Bar"sv) ==
           std::array{"Foo Bar"sv, " Baz"sv});
   REQUIRE(split_first_of_inclusive(input, ","sv) == std::array{input, ""sv});
}

TEST_CASE("string indention", "[Utility][StringOp]")
{
   const auto input = "Foo\nBar\n"sv;
   const auto expected_input = "   Foo\n   Bar\n"sv;

   REQUIRE(indent(1, input, "   "sv) == expected_input);
}

}