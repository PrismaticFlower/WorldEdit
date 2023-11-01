#include "pch.h"

#include "assets/terrain/dirty_rect_tracker.hpp"

using namespace std::literals;

namespace we::assets::terrain::tests {

TEST_CASE("terrain dirty_rect_tracker base", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.x = 0, .y = 0, .width = 8, .height = 8});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.x = 0, .y = 0, .width = 8, .height = 8});

   tracker.add({.x = 16, .y = 16, .width = 8, .height = 8});

   REQUIRE(tracker.size() == 2);

   CHECK(tracker[0] == dirty_rect{.x = 0, .y = 0, .width = 8, .height = 8});
   CHECK(tracker[1] == dirty_rect{.x = 16, .y = 16, .width = 8, .height = 8});

   tracker.clear();

   CHECK(tracker.size() == 0);
}

TEST_CASE("terrain dirty_rect_tracker overlap", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.x = 0, .y = 0, .width = 8, .height = 8});
   tracker.add({.x = 4, .y = 4, .width = 8, .height = 8});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.x = 0, .y = 0, .width = 12, .height = 12});
}

TEST_CASE("terrain dirty_rect_tracker edge overlap", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.x = 0, .y = 0, .width = 8, .height = 8});
   tracker.add({.x = 8, .y = 0, .width = 8, .height = 8});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.x = 0, .y = 0, .width = 16, .height = 8});
}

}
