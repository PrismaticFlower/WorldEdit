#pragma once

#include "pch.h"

#include "async/for_each.hpp"

#include <algorithm>
#include <array>

using namespace std::literals;

namespace we::async::tests {

TEST_CASE("async for_each", "[Async][ThreadPool]")
{

   auto thread_pool =
      thread_pool::make({.thread_count = 0, .low_priority_thread_count = 0});

   std::array<int, 16> arr{};

   for_each(*thread_pool, async::task_priority::normal, arr,
            [&](int& v) noexcept { v += 1; });

   REQUIRE(std::ranges::all_of(arr, [](int v) { return v == 1; }));
}

}
