#include "pch.h"

#include "container/ring_set.hpp"

namespace we::container::tests {

TEST_CASE("ring_set default construct", "[Container][RingSet]")
{
   ring_set<int, 8> set;

   REQUIRE(set.empty());
   REQUIRE(set.size() == 0);
}

TEST_CASE("ring_set insert", "[Container][RingSet]")
{
   ring_set<int, 8> set;

   CHECK(set.insert(128) == 128);
   CHECK(set.size() == 1);
}

TEST_CASE("ring_set insert deduplicate", "[Container][RingSet]")
{
   ring_set<int, 8> set;

   CHECK(set.insert(128) == 128);
   CHECK(set.insert(128) == 128);
   CHECK(set.size() == 1);
}

TEST_CASE("ring_set index", "[Container][RingSet]")
{
   ring_set<int, 8> set;

   CHECK(set.insert(128) == 128);
   CHECK(set[0] == 128);
}

TEST_CASE("ring_set insert wrap", "[Container][RingSet]")
{
   ring_set<int, 8> set;

   for (int i = 0; i < 10; ++i) set.insert(i);

   CHECK(set[0] == 8);
   CHECK(set[1] == 9);
   CHECK(set[2] == 2);
   CHECK(set[3] == 3);
   CHECK(set[4] == 4);
   CHECK(set[5] == 5);
   CHECK(set[6] == 6);
   CHECK(set[7] == 7);
}

}
