#include "pch.h"

#include "lowercase_string.hpp"

using namespace std::literals;

namespace sk::tests {

TEST_CASE("lowercase string tests", "[String]")
{
   REQUIRE(lowercase_string{"hEllO wORLd!"s} == "hello world!"s);
   REQUIRE(lowercase_string{"STRING!"sv} == "string!"sv);
   REQUIRE(lowercase_string{"hEllO wORLd!"s} == lowercase_string{"HELLO world!"s});

   REQUIRE(std::hash<lowercase_string>{}(lowercase_string{"hEllO wORLd!"s}) ==
           std::hash<std::string>{}("hello world!"s));
}

}
