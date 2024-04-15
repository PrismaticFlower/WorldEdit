#include "pch.h"

#include "utility/string_icompare.hpp"

using namespace std::literals;

namespace we::string::tests {

TEST_CASE("string iequals test", "[Utility]")
{
   REQUIRE(iequals("Hello"sv, "hElLo"sv));
   REQUIRE(not iequals("Hello!"sv, "hElLo?"sv));
}

TEST_CASE("string icompare test", "[Utility]")
{
   REQUIRE(icompare("Apple"sv, "Apple"sv) == 0);

   REQUIRE(icompare("AAA"sv, "aaB"sv) < 0);
   REQUIRE(icompare("AAB"sv, "AaA"sv) > 0);

   REQUIRE(icompare("How are you???"sv, "How are you?"sv) > 0);
   REQUIRE(icompare("How are YOU???"sv, "How ARE you?"sv) > 0);

   REQUIRE(icompare("How are you?"sv, "How are you???"sv) < 0);
   REQUIRE(icompare("HOW are you?"sv, "How are YOU???"sv) < 0);
}

TEST_CASE("string iless_than test", "[Utility]")
{
   REQUIRE(not iless_than("Apple"sv, "Apple"sv));

   REQUIRE(iless_than("AAA"sv, "aaB"sv));
   REQUIRE(not iless_than("AAB"sv, "AaA"sv));

   REQUIRE(not iless_than("How are you???"sv, "How are you?"sv));
   REQUIRE(not iless_than("How are YOU???"sv, "How ARE you?"sv));

   REQUIRE(iless_than("How are you?"sv, "How are you???"sv));
   REQUIRE(iless_than("HOW are you?"sv, "How are YOU???"sv));
}

TEST_CASE("string igreater_than test", "[Utility]")
{
   REQUIRE(not igreater_than("Apple"sv, "Apple"sv));

   REQUIRE(not igreater_than("AAA"sv, "aaB"sv));
   REQUIRE(igreater_than("AAB"sv, "AaA"sv));

   REQUIRE(igreater_than("How are you???"sv, "How are you?"sv));
   REQUIRE(igreater_than("How are YOU???"sv, "How ARE you?"sv));

   REQUIRE(not igreater_than("How are you?"sv, "How are you???"sv));
   REQUIRE(not igreater_than("HOW are you?"sv, "How are YOU???"sv));
}

TEST_CASE("string istarts_with test", "[Utility]")
{
   REQUIRE(istarts_with("How are you?"sv, "How are YOU?"sv));
   REQUIRE(istarts_with("How are you?"sv, "How ARE"sv));
   REQUIRE(not istarts_with("How are you?"sv, "Now NOT"sv));
   REQUIRE(not istarts_with("How are you?"sv,
                            "How are you? A really long string. Isn't that fun?"sv));
}

TEST_CASE("string iends_with test", "[Utility]")
{
   REQUIRE(iends_with("How are you?"sv, "How are YOU?"sv));
   REQUIRE(iends_with("How are you?"sv, "YOU?"sv));
   REQUIRE(not iends_with("How are you?"sv, "you!"sv));
   REQUIRE(not iends_with("How are you?"sv,
                          "How are you? A really long string. Isn't that fun?"sv));
}

TEST_CASE("string icontains test", "[Utility]")
{
   REQUIRE(icontains("How are you?"sv, "How are YOU?"sv));
   REQUIRE(icontains("How are you?"sv, "ARE"sv));
   REQUIRE(not icontains("How are you?"sv, "you!"sv));
   REQUIRE(not icontains("How are you?"sv, "bye?"sv));
   REQUIRE(not icontains("How are you?"sv,
                         "How are you? A really long string. Isn't that fun?"sv));
}

TEST_CASE("string wide iequals test", "[Utility]")
{
   REQUIRE(iequals(L"Hello"sv, L"hElLo"sv));
   REQUIRE(not iequals(L"Hello!"sv, L"hElLo?"sv));
}

}