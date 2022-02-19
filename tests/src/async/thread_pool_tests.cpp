#include "pch.h"

#include "async/thread_pool.hpp"

using namespace std::literals;

namespace we::async::tests {

TEST_CASE("async detail task_context_base", "[Async][ThreadPool]")
{
   // these are very meh but something is surely better than nothing

   detail::task_context_base context{
      .execute_function = [&context] { context.executed_latch.count_down(); }};

   SECTION("basic tests")
   {
      REQUIRE(not context.ready());
      REQUIRE(not context.execution_started.load());

      REQUIRE(context.try_direct_execute());
      REQUIRE(context.execution_started.load());
      REQUIRE(context.ready());
   }

   SECTION("basic unready tests")
   {
      REQUIRE(not context.ready());
      REQUIRE(not context.execution_started.load());

      context.execution_started.store(true); // fake async execution

      REQUIRE(not context.try_direct_execute());
      REQUIRE(context.execution_started.load());
      REQUIRE(not context.ready());
   }
}

TEST_CASE("async task default construct", "[Async][ThreadPool]")
{
   task<int> task0;
   task<void> task1;

   REQUIRE(not task0.valid());
   REQUIRE(not task1.valid());
}

TEST_CASE("async task typed", "[Async][ThreadPool]")
{
   std::shared_ptr context = std::make_shared<detail::task_context<int>>();

   context->execute_function = [&] {
      context->result = 5;
      context->executed_latch.count_down();
   };
   context->owning_thread_pool = {}; // we have no thread_pool to assign here so cancel can not be tested here

   task<int> task{context};

   REQUIRE(task.valid());
   REQUIRE(not task.ready());

   SECTION("indirect (async) execution")
   {
      context->execute_function();

      REQUIRE(task.ready());
      REQUIRE(task.get() == 5);
   }

   SECTION("direct execution")
   {
      REQUIRE(task.get() == 5);
   }
}

TEST_CASE("async task typed exception", "[Async][ThreadPool]")
{
   std::shared_ptr context = std::make_shared<detail::task_context<int>>();

   context->execute_function = [&] {
      context->task_exception_ptr =
         std::make_exception_ptr(std::runtime_error{"exception"});
      context->executed_latch.count_down();
   };
   context->owning_thread_pool = {}; // we have no thread_pool to assign here so cancel can not be tested here

   task<int> task{context};

   REQUIRE(task.valid());
   REQUIRE(not task.ready());

   SECTION("indirect (async) execution")
   {
      context->execute_function();

      REQUIRE(task.ready());
      REQUIRE_THROWS(task.get());
   }

   SECTION("direct execution")
   {
      REQUIRE_THROWS(task.get());
   }
}

TEST_CASE("async task typed wait", "[Async][ThreadPool]")
{
   std::shared_ptr context = std::make_shared<detail::task_context<int>>();

   int task_invoked = 0;

   context->execute_function = [&] {
      task_invoked += 1;
      context->executed_latch.count_down();

      return 0;
   };
   context->owning_thread_pool = {}; // we have no thread_pool to assign here so cancel can not be tested here

   task<int> task{context};

   REQUIRE(task.valid());
   REQUIRE(not task.ready());

   task.wait();

   REQUIRE(task.ready());

   task.wait(); // waiting mutliple times should cause no issues

   REQUIRE(task_invoked == 1);
   REQUIRE(task.get() == 0);
}

TEST_CASE("async task void", "[Async][ThreadPool]")
{
   std::shared_ptr context = std::make_shared<detail::task_context<void>>();

   bool task_invoked = false;

   context->execute_function = [&] {
      task_invoked = true;
      context->executed_latch.count_down();
   };
   context->owning_thread_pool = {}; // we have no thread_pool to assign here so cancel can not be tested here

   task<void> task{context};

   REQUIRE(task.valid());
   REQUIRE(not task.ready());

   SECTION("indirect (async) execution")
   {
      context->execute_function();

      REQUIRE(task.ready());

      task.get(); // not strictly needed here but shows correct usage for tasks returned from thread_pool

      REQUIRE(task_invoked);
   }

   SECTION("direct execution")
   {
      task.get();

      REQUIRE(task_invoked);
   }
}

TEST_CASE("async task void exception", "[Async][ThreadPool]")
{
   std::shared_ptr context = std::make_shared<detail::task_context<void>>();

   context->execute_function = [&] {
      context->task_exception_ptr =
         std::make_exception_ptr(std::runtime_error{"exception"});
      context->executed_latch.count_down();
   };
   context->owning_thread_pool = {}; // we have no thread_pool to assign here so cancel can not be tested here

   task<void> task{context};

   REQUIRE(task.valid());
   REQUIRE(not task.ready());

   SECTION("indirect async execution")
   {
      context->execute_function();

      REQUIRE(task.ready());
      REQUIRE_THROWS(task.get());
   }

   SECTION("direct execution")
   {
      REQUIRE_THROWS(task.get());
   }
}

TEST_CASE("async task void wait", "[Async][ThreadPool]")
{
   std::shared_ptr context = std::make_shared<detail::task_context<void>>();

   int task_invoked = 0;

   context->execute_function = [&] {
      task_invoked += 1;
      context->executed_latch.count_down();

      return 0;
   };
   context->owning_thread_pool = {}; // we have no thread_pool to assign here so cancel can not be tested here

   task<void> task{context};

   REQUIRE(task.valid());
   REQUIRE(not task.ready());

   task.wait();

   REQUIRE(task.ready());

   task.wait(); // waiting mutliple times should cause no issues

   REQUIRE(task_invoked == 1);
}

TEST_CASE("async thread_pool", "[Async][ThreadPool]")
{
   auto thread_pool =
      thread_pool::make({.thread_count = 2, .low_priority_thread_count = 2});

   REQUIRE(thread_pool->thread_count(task_priority::low) == 2);
   REQUIRE(thread_pool->thread_count(task_priority::normal) == 2);

   task<int> get_32 = thread_pool->exec(task_priority::normal, [] { return 32; });
   task<int> get_exception = thread_pool->exec(task_priority::normal, []() -> int {
      throw std::runtime_error{"Hello!"};
   });

   // Verify the tasks run in the background.

   while (not get_32.ready())
      ;

   REQUIRE(get_32.get() == 32);

   while (not get_exception.ready())
      ;

   REQUIRE_THROWS(get_exception.get());

   // Verify that void tasks work as well.

   bool void_task_called = false;

   task<void> void_task =
      thread_pool->exec(task_priority::normal,
                        [&void_task_called] { void_task_called = true; });
   task<void> void_task_exception = thread_pool->exec(task_priority::normal, [] {
      throw std::runtime_error{"Hello!"};
   });

   void_task.wait();

   REQUIRE(void_task_called);
   REQUIRE_THROWS(void_task_exception.get());
}

TEST_CASE("async thread_pool singlethreaded mode", "[Async][ThreadPool]")
{
   auto thread_pool =
      thread_pool::make({.thread_count = 0, .low_priority_thread_count = 0});

   REQUIRE(thread_pool->thread_count(task_priority::low) == 0);
   REQUIRE(thread_pool->thread_count(task_priority::normal) == 0);

   task<int> get_32 = thread_pool->exec(task_priority::normal, [] { return 32; });
   task<int> get_exception = thread_pool->exec(task_priority::normal, []() -> int {
      throw std::runtime_error{"Hello!"};
   });
   bool void_task_called = false;

   task<void> void_task =
      thread_pool->exec(task_priority::normal,
                        [&void_task_called] { void_task_called = true; });
   task<void> void_task_exception = thread_pool->exec(task_priority::normal, [] {
      throw std::runtime_error{"Hello!"};
   });

   REQUIRE(get_32.ready());
   REQUIRE(get_32.get() == 32);

   REQUIRE(get_exception.ready());
   REQUIRE_THROWS(get_exception.get());

   REQUIRE(void_task.ready());
   REQUIRE(void_task_called);

   REQUIRE(void_task_exception.ready());
   REQUIRE_THROWS(void_task_exception.get());
}

}
