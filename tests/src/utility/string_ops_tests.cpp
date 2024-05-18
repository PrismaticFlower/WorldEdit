#include "pch.h"

#include "utility/string_ops.hpp"

using namespace std::literals;

namespace we::string::tests {

TEST_CASE("string count lines", "[Utility][StringOp]")
{
   const auto input = "Line0\nLine1\nLine2\n"sv;

   REQUIRE(count_lines(input) == 3);
}

TEST_CASE("string lines iterator", "[Utility][StringOp]")
{
   const auto input = "Line0\nLine1\r\nLine2\n\n"sv;
   const std::array expected_lines{"Line0"sv, "Line1"sv, "Line2"sv, ""sv};

   int i = 0;
   for (auto line : lines_iterator{input}) {
      CHECK(line.number == i + 1);
      CHECK(line.string == expected_lines.at(i));

      ++i;
   }

   REQUIRE(i == expected_lines.size());
}

TEST_CASE("string split first of inclusive", "[Utility][StringOp]")
{
   const auto input = "Foo Bar Baz"sv;

   REQUIRE(split_first_of_inclusive(input, "Bar"sv) ==
           std::array{"Foo Bar"sv, " Baz"sv});
   REQUIRE(split_first_of_inclusive(input, ","sv) == std::array{input, ""sv});
}

TEST_CASE("string split first of exclusive", "[Utility][StringOp]")
{
   const auto input = "Foo Bar Baz"sv;

   REQUIRE(split_first_of_exclusive(input, "Bar"sv) == std::array{"Foo "sv, " Baz"sv});
   REQUIRE(split_first_of_exclusive(input, ","sv) == std::array{input, ""sv});
}

TEST_CASE("string split first of exclusive whitespace", "[Utility][StringOp]")
{
   const auto input = "Foo Bar Baz"sv;

   REQUIRE(split_first_of_exclusive_whitespace(input) ==
           std::array{"Foo"sv, "Bar Baz"sv});
   REQUIRE(split_first_of_exclusive_whitespace("FooBarBaz"sv) ==
           std::array{"FooBarBaz"sv, ""sv});
}

TEST_CASE("string split first of right inclusive any", "[Utility][StringOp]")
{
   const auto input = "Foo Bar Baz"sv;

   REQUIRE(split_first_of_right_inclusive_any(input, {"Bar"sv}) ==
           std::array{"Foo "sv, "Bar Baz"sv});
   REQUIRE(split_first_of_right_inclusive_any(input, {"o"sv, " "sv}) ==
           std::array{"F"sv, "oo Bar Baz"sv});
   REQUIRE(split_first_of_right_inclusive_any(input, {"B"sv, " "sv}) ==
           std::array{"Foo"sv, " Bar Baz"sv});
   REQUIRE(split_first_of_right_inclusive_any(input, {","sv}) ==
           std::array{input, ""sv});
}

TEST_CASE("string trim leading whitespace", "[Utility][StringOp]")
{
   const auto input = "   \f\v\r\n\n\tFoo Bar"sv;

   REQUIRE(trim_leading_whitespace(input) == "Foo Bar"sv);
}

TEST_CASE("string trim trailing whitespace", "[Utility][StringOp]")
{
   const auto input = "Foo Bar   \f\v\r\n\n\t"sv;

   REQUIRE(trim_trailing_whitespace(input) == "Foo Bar"sv);
}

TEST_CASE("string trim trailing digits", "[Utility][StringOp]")
{
   const auto input = "Foo32Bar1_64"sv;

   REQUIRE(trim_trailing_digits(input) == "Foo32Bar1_"sv);
}

TEST_CASE("string trim whitespace", "[Utility][StringOp]")
{
   const auto input = "   \f\v\r\n\n\tFoo Bar   \f\v\r\n\n\t"sv;

   REQUIRE(trim_whitespace(input) == "Foo Bar"sv);
}

TEST_CASE("string is whitespace", "[Utility][StringOp]")
{
   REQUIRE(is_whitespace("   \f\v\r\n\n\t"sv));
   REQUIRE(not is_whitespace("Foo Bar"sv));
}

TEST_CASE("string substr distance", "[Utility][StringOp]")
{
   const auto input = "Foo Baz"sv;

   REQUIRE(substr_distance(input, input.substr(3)) == 3);

   // The following would invoke Undefined Behaviour, by subtracting pointers from two different arrays.
   // aka do not do this, make sure you call substr_distance with a valid substr.
   //
   // REQUIRE(substr_distance(input,  " Baz"sv) == 3);
}
TEST_CASE("string indention", "[Utility][StringOp]")
{
   const auto input = "Foo\nBar\n"sv;
   const auto expected_input = "   Foo\n   Bar\n"sv;

   REQUIRE(indent(1, input, "   "sv) == expected_input);
}

}
