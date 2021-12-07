#include "pch.h"

#include "utility/command_line.hpp"

using namespace std::literals;

namespace we::utility::tests {

TEST_CASE("command_line tests", "[Utility][CommandLine]")
{
   const std::array arguments =
      {"Test Program Path", "-intval",   "457838",
       "not a number",      "-floatval", "50.0",
       "-stringval",        "Hello!",    "-missingval"};

   command_line command_line{static_cast<int>(arguments.size()), arguments.data()};

   // check correct use
   CHECK(command_line.get_or("-intval", 0) == 457838);
   CHECK(command_line.get_or("-floatval", 0.0) == 50.0);
   CHECK(command_line.get_or("-stringval", "fallback") == "Hello!"sv);

   // check type mismatch fallback use
   CHECK(command_line.get_or("-stringval", 32) == 32);
   CHECK(command_line.get_or("-stringval", 32.0) == 32.0);

   // getting a string should always work
   CHECK(command_line.get_or("-intval", "fallback") == "457838"sv);
   CHECK(command_line.get_or("-floatval", "fallback") == "50.0"sv);
   CHECK(command_line.get_or("-stringval", "fallback") == "Hello!"sv);

   // and treating a simple float as an int also works, so may as well test for it
   CHECK(command_line.get_or("-floatval", 0) == 50.0);

   // getting a missing value should return the fallback
   CHECK(command_line.get_or("-missingval", "fallback") == "fallback"sv);
}

}
