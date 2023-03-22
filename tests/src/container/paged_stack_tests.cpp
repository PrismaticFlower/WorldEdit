#include "pch.h"

#include "container/paged_stack.hpp"

namespace we::container::tests {

TEST_CASE("paged_stack push/pop test", "[Container][PagedStack]")
{
   paged_stack<int, 2> stack;

   for (int i = 0; i < 16; ++i) {
      CHECK(stack.push(i) == i);
   }

   REQUIRE(stack.size() == 16);

   for (int i = 15; i >= 0; --i) {
      CHECK(stack.pop() == i);
   }
}

TEST_CASE("paged_stack emplace/pop test", "[Container][PagedStack]")
{
   paged_stack<int, 2> stack;

   for (int i = 0; i < 16; ++i) {
      CHECK(stack.emplace(i) == i);
   }

   REQUIRE(stack.size() == 16);

   for (int i = 15; i >= 0; --i) {
      CHECK(stack.pop() == i);
   }
}

TEST_CASE("paged_stack empty test", "[Container][PagedStack]")
{
   paged_stack<int, 2> stack;

   REQUIRE(stack.empty());

   stack.push(0);

   REQUIRE(not stack.empty());
}

TEST_CASE("paged_stack top test", "[Container][PagedStack]")
{
   paged_stack<int, 2> stack;

   REQUIRE(stack.push(1) == stack.top());
}

TEST_CASE("paged_stack clear test", "[Container][PagedStack]")
{
   paged_stack<int, 2> stack;

   for (int i = 0; i < 16; ++i) stack.push(i);

   REQUIRE(not stack.empty());

   stack.clear();

   REQUIRE(stack.empty());

   for (int i = 0; i < 16; ++i) {
      CHECK(stack.push(i) == i);
   }

   REQUIRE(stack.size() == 16);

   for (int i = 15; i >= 0; --i) {
      CHECK(stack.pop() == i);
   }
}

}
