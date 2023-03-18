#include "pch.h"

#include "utility/string_icompare.hpp"

using namespace std::literals;

namespace we::string::tests {

TEST_CASE("string iequals test", "[Utility][StopWatch]")
{
   REQUIRE(iequals("Hello"sv, "hElLo"sv));
   REQUIRE(not iequals("Hello!"sv, "hElLo?"sv));
}

TEST_CASE("string istarts_with test", "[Utility][StopWatch]")
{
   REQUIRE(istarts_with("How are you?"sv, "How are YOU?"sv));
   REQUIRE(istarts_with("How are you?"sv, "How ARE"sv));
   REQUIRE(not istarts_with("How are you?"sv, "Now NOT"sv));
   REQUIRE(not istarts_with("How are you?"sv,
                            "How are you? A really long string. Isn't that fun?"sv));
}

TEST_CASE("string iends_with test", "[Utility][StopWatch]")
{
   REQUIRE(iends_with("How are you?"sv, "How are YOU?"sv));
   REQUIRE(iends_with("How are you?"sv, "YOU?"sv));
   REQUIRE(not iends_with("How are you?"sv, "you!"sv));
   REQUIRE(not iends_with("How are you?"sv,
                          "How are you? A really long string. Isn't that fun?"sv));
}

}