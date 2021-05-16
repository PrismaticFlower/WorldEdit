#include "pch.h"

#include "utility/event.hpp"

namespace sk::utility::tests {

TEST_CASE("event listener tests", "[Utility][Event]")
{
   event<void(int)> event;

   int value = 0;
   int count = 0;

   const auto callback = [&](int v) {
      value = v;
      ++count;
   };

   event_listener<void(int)> listener = event.listen(callback);

   event.broadcast(1);

   REQUIRE(value == 1);
   REQUIRE(count == 1);

   listener = event_listener<void(int)>{};

   event.broadcast(2);

   REQUIRE(value == 1);
   REQUIRE(count == 1);

   listener = event.listen(callback);

   event.broadcast(3);

   REQUIRE(value == 3);
   REQUIRE(count == 2);

   event_listener<void(int)> other_listener{std::move(listener)};

   event.broadcast(4);

   REQUIRE(value == 4);
   REQUIRE(count == 3);

   listener = event.listen(callback);

   event.broadcast(5);

   REQUIRE(value == 5);
   REQUIRE(count == 5);
}

TEST_CASE("event tests", "[Utility][Event]")
{
   event<void()> test_event;

   int count = 0;

   const auto callback = [&]() { ++count; };

   event_listener<void()> listener = test_event.listen(callback);

   SECTION("overwrite")
   {
      test_event.broadcast();

      REQUIRE(count == 1);

      test_event = event<void()>{};

      test_event.broadcast();

      REQUIRE(count == 1);
   }

   SECTION("move into")
   {
      event<void()> other_event = std::move(test_event);

      other_event.broadcast();

      REQUIRE(count == 1);
   }

   SECTION("swap")
   {
      event<void()> other_event;

      test_event.broadcast();

      REQUIRE(count == 1);

      other_event.broadcast();

      REQUIRE(count == 1);

      std::swap(test_event, other_event);

      other_event.broadcast();

      REQUIRE(count == 2);
   }
}

}
