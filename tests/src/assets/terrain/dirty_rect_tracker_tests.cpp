#include "pch.h"

#include "assets/terrain/dirty_rect_tracker.hpp"

using namespace std::literals;

namespace we::assets::terrain::tests {

TEST_CASE("terrain dirty_rect_tracker separate", "[Assets][Terrain]")
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

TEST_CASE("terrain dirty_rect_tracker contained by old", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});
   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});
   tracker.add({.left = 0, .top = 0, .right = 4, .bottom = 4});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
}

TEST_CASE("terrain dirty_rect_tracker contained by new", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 2, .bottom = 2});
   tracker.add({.left = 0, .top = 0, .right = 4, .bottom = 2});
   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
}

TEST_CASE("terrain dirty_rect_tracker overlap bottom right",
          "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});
   tracker.add({.left = 4, .top = 4, .right = 12, .bottom = 12});

   REQUIRE(tracker.size() == 3);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 4, .top = 8, .right = 12, .bottom = 12});
   CHECK(tracker[2] == dirty_rect{.left = 8, .top = 4, .right = 12, .bottom = 8});
}

TEST_CASE("terrain dirty_rect_tracker overlap right", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});
   tracker.add({.left = 4, .top = 0, .right = 12, .bottom = 7});

   REQUIRE(tracker.size() == 2);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 8, .top = 0, .right = 12, .bottom = 7});
}

TEST_CASE("terrain dirty_rect_tracker overlap bottom", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});
   tracker.add({.left = 0, .top = 4, .right = 7, .bottom = 12});

   REQUIRE(tracker.size() == 2);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 8, .right = 7, .bottom = 12});
}

TEST_CASE("terrain dirty_rect_tracker overlap top left", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 4, .top = 4, .right = 12, .bottom = 12});
   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});

   REQUIRE(tracker.size() == 3);

   CHECK(tracker[0] == dirty_rect{.left = 4, .top = 4, .right = 12, .bottom = 12});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 4});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 4, .right = 4, .bottom = 8});
}

TEST_CASE("terrain dirty_rect_tracker overlap left", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 4, .top = 0, .right = 12, .bottom = 8});
   tracker.add({.left = 0, .top = 0, .right = 5, .bottom = 4});

   REQUIRE(tracker.size() == 2);

   CHECK(tracker[0] == dirty_rect{.left = 4, .top = 0, .right = 12, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 4, .bottom = 4});
}

TEST_CASE("terrain dirty_rect_tracker overlap top", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 4, .right = 8, .bottom = 12});
   tracker.add({.left = 0, .top = 0, .right = 4, .bottom = 5});

   REQUIRE(tracker.size() == 2);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 4, .right = 8, .bottom = 12});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 4, .bottom = 4});
}

TEST_CASE("terrain dirty_rect_tracker overlap small left", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 4, .top = 2, .right = 10, .bottom = 6});
   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});

   REQUIRE(tracker.size() == 3);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 2});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 6, .right = 8, .bottom = 8});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 10, .bottom = 6});
}

TEST_CASE("terrain dirty_rect_tracker overlap small right", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 2, .right = 4, .bottom = 6});
   tracker.add({.left = 2, .top = 0, .right = 10, .bottom = 8});

   REQUIRE(tracker.size() == 3);

   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 0, .right = 10, .bottom = 2});
   CHECK(tracker[1] == dirty_rect{.left = 2, .top = 6, .right = 10, .bottom = 8});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 10, .bottom = 6});
}

TEST_CASE("terrain dirty_rect_tracker overlap small top", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 2, .top = 2, .right = 6, .bottom = 10});
   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});

   REQUIRE(tracker.size() == 4);

   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 2, .right = 6, .bottom = 10});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 2});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 2, .bottom = 8});
   CHECK(tracker[3] == dirty_rect{.left = 6, .top = 2, .right = 8, .bottom = 8});
}

TEST_CASE("terrain dirty_rect_tracker overlap small bottom",
          "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 2, .top = 0, .right = 6, .bottom = 8});
   tracker.add({.left = 0, .top = 2, .right = 8, .bottom = 10});

   REQUIRE(tracker.size() == 4);

   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 0, .right = 6, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 8, .right = 8, .bottom = 10});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 2, .bottom = 8});
   CHECK(tracker[3] == dirty_rect{.left = 6, .top = 2, .right = 8, .bottom = 8});
}

TEST_CASE("terrain dirty_rect_tracker overlap top 3 sides", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({8, 16, 24, 24});
   tracker.add({8, 8, 12, 24});
   tracker.add({20, 8, 24, 24});
   tracker.add({11, 8, 21, 18});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{8, 8, 24, 24});
}

TEST_CASE("terrain dirty_rect_tracker overlap bottom 3 sides",
          "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({8, 8, 24, 16});
   tracker.add({8, 8, 12, 24});
   tracker.add({20, 8, 24, 24});
   tracker.add({11, 14, 21, 24});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{8, 8, 24, 24});
}

TEST_CASE("terrain dirty_rect_tracker overlap left 3 sides",
          "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({16, 8, 24, 24});
   tracker.add({8, 8, 24, 12});
   tracker.add({8, 20, 24, 24});
   tracker.add({8, 11, 18, 21});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{8, 8, 24, 24});
}

TEST_CASE("terrain dirty_rect_tracker overlap right 3 sides",
          "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({8, 8, 16, 24});
   tracker.add({8, 8, 24, 12});
   tracker.add({8, 20, 24, 24});
   tracker.add({14, 11, 24, 21});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{8, 8, 24, 24});
}

TEST_CASE("terrain dirty_rect_tracker edge join", "[Assets][Terrain]")
{
   dirty_rect_tracker tracker;

   REQUIRE(tracker.size() == 0);

   tracker.add({.left = 0, .top = 0, .right = 8, .bottom = 8});
   tracker.add({.left = 8, .top = 0, .right = 16, .bottom = 8});

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 16, .bottom = 8});
}

}
