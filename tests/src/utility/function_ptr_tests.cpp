#include "pch.h"

#include "types.hpp"
#include "utility/function_ptr.hpp"

using namespace Catch::literals;

namespace we::tests {

namespace {

int add(int x, int y)
{
   return x + y;
}

}

TEST_CASE("function_ptr", "[Utility][FunctionPtr]")
{
   CHECK(function_ptr<int(int, int) noexcept>{
            [](int x, int y) noexcept -> int { return x + y; }}(2, 2) == 4);
   CHECK(function_ptr<int(int, int) noexcept>{add}(2, 2) == 4);
   CHECK(function_ptr<int(int, int) noexcept>{[bonus = 1](int x, int y) noexcept -> int {
            return x + y + bonus;
         }}(2, 2) == 5);
   CHECK(not function_ptr<int(int, int) noexcept>{nullptr});
   CHECK(function_ptr<int(int, int) noexcept>{nullptr} == nullptr);
}

}
