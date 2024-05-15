#include "pch.h"

#include "utility/stopwatch.hpp"

#include <thread>

using namespace std::literals;

namespace we::utility::tests {

TEST_CASE("stopwatch tests", "[Utility][StopWatch]")
{
   utility::stopwatch stopwatch;

   std::this_thread::sleep_for(1ms); // ... It's not great but I mean, how else can you test this?

   REQUIRE(stopwatch.elapsed_ms() >= 1.0f);
}

}