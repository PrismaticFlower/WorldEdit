#pragma once

#include "pch.h"

#include "async/wait_all.hpp"

#include <array>

using namespace std::literals;

namespace we::async::tests {

namespace {

// Helper to get and then discard the return value from the tasks.
void discard_results(auto&... tasks)
{
   ((void)tasks.get(), ...);
}

}

TEST_CASE("async wait_all", "[Async][ThreadPool]")
{
   auto thread_pool =
      thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});

   task<int> task0 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task1 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task2 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task3 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task4 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task5 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task6 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task7 = thread_pool->exec(task_priority::normal, [] { return 1; });
   task<int> task8 = thread_pool->exec(task_priority::normal, [] { return 1; });

   wait_all(task0, task1, task2, task3, task4, task5, task6, task7, task8);

   REQUIRE(task0.ready());
   REQUIRE(task1.ready());
   REQUIRE(task2.ready());
   REQUIRE(task3.ready());
   REQUIRE(task4.ready());
   REQUIRE(task5.ready());
   REQUIRE(task6.ready());
   REQUIRE(task7.ready());
   REQUIRE(task8.ready());

   discard_results(task0, task1, task2, task3, task4, task5, task6, task7, task8);
}

TEST_CASE("async wait_all single", "[Async][ThreadPool]")
{
   auto thread_pool =
      thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});

   task<int> task = thread_pool->exec(task_priority::normal, [] { return 1; });

   wait_all(task);

   REQUIRE(task.ready());

   discard_results(task);
}

TEST_CASE("async wait_all range", "[Async][ThreadPool]")
{
   auto thread_pool =
      thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});

   std::array tasks{thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 1; }),
                    thread_pool->exec(task_priority::normal, [] { return 1; })};

   wait_all(tasks);

   REQUIRE(tasks[0].ready());
   REQUIRE(tasks[1].ready());
   REQUIRE(tasks[2].ready());
   REQUIRE(tasks[3].ready());
   REQUIRE(tasks[4].ready());
   REQUIRE(tasks[5].ready());
   REQUIRE(tasks[6].ready());
   REQUIRE(tasks[7].ready());
   REQUIRE(tasks[8].ready());

   discard_results(tasks[0], tasks[1], tasks[2], tasks[3], tasks[4], tasks[5],
                   tasks[6], tasks[7], tasks[8]);
}

}
