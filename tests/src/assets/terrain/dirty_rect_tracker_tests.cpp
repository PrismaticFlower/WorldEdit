#include "pch.h"

#include "assets/terrain/dirty_rect_tracker.hpp"

using namespace std::literals;

namespace we::assets::terrain::tests {

TEST_CASE("terrain dirty_rect_tracker base", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});

   tracker.add({.left = 16, .top = 16, .right = 24, .bottom = 24});

   REQUIRE(tracker.size() == 2);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 16, .top = 16, .right = 24, .bottom = 24});

   tracker.clear();

   CHECK(tracker.size() == 0);
}

TEST_CASE("terrain dirty_rect_tracker overlap", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});
   tracker.add({.left = 4, .top = 4, .right = 12, .bottom = 12});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 12, .bottom = 12});
}

TEST_CASE("terrain dirty_rect_tracker edge overlap", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});
   tracker.add({.left = 8, .top = 0, .right = 16, .bottom = 8});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 16, .bottom = 8});
}

}
