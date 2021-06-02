#include "pch.h"

#include "assets/config/values.hpp"

#include <string_view>

using namespace std::literals;
using namespace Catch::literals;

namespace we::assets::config::tests {

TEST_CASE("config values get/set", "[Assets][Config]")
{
   values test_values = {"Hello", 8, 50.0, 21979ll};

   CHECK(test_values.get<std::string>(0) == "Hello"sv);
   CHECK(test_values.get<std::string_view>(0) == "Hello"sv);
   CHECK_THROWS(test_values.get<int>(0));

   CHECK(test_values.get<short>(1) == 8);
   CHECK(test_values.get<int>(1) == 8);
   CHECK(test_values.get<long long>(1) == 8);
   CHECK(test_values.get<float>(1) == 8.0_a);
   CHECK_THROWS(test_values.get<std::string>(1));

   CHECK(test_values.get<float>(2) == 50.0_a);
   CHECK(test_values.get<double>(2) == 50.0_a);
   CHECK(test_values.get<long long>(2) == 50);
   CHECK_THROWS(test_values.get<std::string>(2));

   CHECK(test_values.get<long long>(3) == 21979);
   CHECK(test_values.get<double>(3) == 21979.0_a);

   test_values.set(1, "8"s);

   CHECK(test_values.get<std::string_view>(1) == "8"sv);
   CHECK_THROWS(test_values.get<int>(1));

   CHECK_THROWS(test_values.get<int>(7657));
   CHECK_THROWS(test_values.set(7657, 3.0f));
}
}
