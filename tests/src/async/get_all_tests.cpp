#pragma once

#include "pch.h"

#include "async/get_all.hpp"

#include <array>

using namespace std::literals;

namespace we::async::tests {

TEST_CASE("async get_all", "[Async][ThreadPool]")
{
   auto thread_pool =
      thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});

   task<int> task0 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task1 = thread_pool->exec(task_priority::normal, [] { return 2; });
   task<int> task2 = thread_pool->exec(task_priority::normal, [] { return 3; });
   task<int> task3 = thread_pool->exec(task_priority::normal, [] { return 4; });
   task<double> task4 =
      thread_pool->exec(task_priority::normal, [] { return 1.0; });
   task<float> task5 =
      thread_pool->exec(task_priority::normal, [] { return 1.0f; });
   task<float> task6 =
      thread_pool->exec(task_priority::normal, [] { return 2.0f; });
   task<float> task7 =
      thread_pool->exec(task_priority::normal, [] { return 3.0f; });
   task<float> task8 =
      thread_pool->exec(task_priority::normal, [] { return 4.0f; });

   REQUIRE(get_all(task0, task1, task2, task3, task4, task5, task6, task7, task8) ==
           std::tuple{1, 2, 3, 4, 1.0, 1.0f, 2.0f, 3.0f, 4.0f});
}

TEST_CASE("async get_all single", "[Async][ThreadPool]")
{
   auto thread_pool =
      thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});

   task<int> task = thread_pool->exec(task_priority::normal, [] { return 1; });

   REQUIRE(get_all(task) == std::tuple{1});
}

TEST_CASE("async get_all range", "[Async][ThreadPool]")
{
   auto thread_pool =
      thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});

   std::array tasks{thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 2; }),
                    thread_pool->exec(task_priority::normal, [] { return 3; }),
                    thread_pool->exec(task_priority::normal, [] { return 4; }),
                    thread_pool->exec(task_priority::normal, [] { return 5; }),
                    thread_pool->exec(task_priority::normal, [] { return 6; }),
                    thread_pool->exec(task_priority::normal, [] { return 7; }),
                    thread_pool->exec(task_priority::normal, [] { return 8; }),
                    thread_pool->exec(task_priority::normal, [] { return 9; })};

   REQUIRE(get_all(tasks) == std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9});
}

}
