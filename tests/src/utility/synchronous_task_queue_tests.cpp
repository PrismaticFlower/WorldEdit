#include "pch.h"

#include "utility/synchronous_task_queue.hpp"

namespace we::utility::tests {

TEST_CASE("synchronous_task_queue tests", "[Utility][SynchronousTaskQueue]")
{
   synchronous_task_queue queue;

   bool called_a = false;
   bool called_b = false;
   bool called_c = false;

   queue.enqueue([&] { called_a = true; });
   queue.enqueue([&] { called_b = true; });
   queue.enqueue([&] { called_c = true; });

   REQUIRE(called_a == false);
   REQUIRE(called_b == false);
   REQUIRE(called_c == false);

   queue.execute();

   REQUIRE(called_a == true);
   REQUIRE(called_b == true);
   REQUIRE(called_c == true);
}

}
