#include "pch.h"

#include "world/blocks/dirty_range_tracker.hpp"

namespace we::world::tests {

TEST_CASE("world blocks dirty_range_tracker", "[World][Blocks]")
{
   blocks_dirty_range_tracker dirt;

   REQUIRE(dirt.size() == 0);

   dirt.add({0, 4});

   REQUIRE(dirt.size() == 1);

   CHECK(dirt[0] == blocks_dirty_range{0, 4});

   dirt.add({4, 5});

   REQUIRE(dirt.size() == 1);

   CHECK(dirt[0] == blocks_dirty_range{0, 5});

   dirt.add({1, 2});

   REQUIRE(dirt.size() == 1);

   CHECK(dirt[0] == blocks_dirty_range{0, 5});

   dirt.add({6, 9});

   REQUIRE(dirt.size() == 2);

   CHECK(dirt[0] == blocks_dirty_range{0, 5});
   CHECK(dirt[1] == blocks_dirty_range{6, 9});

   dirt.add({5, 7});

   REQUIRE(dirt.size() == 1);

   CHECK(dirt[0] == blocks_dirty_range{0, 9});

   dirt.add({10, 11});
   dirt.remove_index(7);

   REQUIRE(dirt.size() == 2);

   CHECK(dirt[0] == blocks_dirty_range{0, 8});
   CHECK(dirt[1] == blocks_dirty_range{9, 10});

   dirt.insert_index(7);

   REQUIRE(dirt.size() == 2);

   CHECK(dirt[0] == blocks_dirty_range{0, 9});
   CHECK(dirt[1] == blocks_dirty_range{10, 11});

   dirt.insert_index(9);

   REQUIRE(dirt.size() == 2);

   CHECK(dirt[0] == blocks_dirty_range{0, 9});
   CHECK(dirt[1] == blocks_dirty_range{11, 12});

   dirt.clear();

   REQUIRE(dirt.size() == 0);
}

}
