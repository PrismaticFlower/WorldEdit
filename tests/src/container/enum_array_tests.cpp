#include "pch.h"

#include "container/enum_array.hpp"

namespace we::container::tests {

namespace {

enum class test_enum {
   a,
   b,
   c,

   count
};

}

TEST_CASE("enum array", "[Container][EnumArray]")
{
   enum_array<int, test_enum> array{{{0, 5, 7}}};

   CHECK(array[test_enum::a] == 0);
   CHECK(array[test_enum::b] == 5);
   CHECK(array[test_enum::c] == 7);
   CHECK(array.at(test_enum::a) == 0);
   CHECK(array.at(test_enum::b) == 5);
   CHECK(array.at(test_enum::c) == 7);

   array[test_enum::a] = 8;
   CHECK(array[test_enum::a] == 8);

   auto other_array = make_enum_array<int, test_enum>(
      {{test_enum::c, 7}, {test_enum::a, 8}, {test_enum::b, 5}});

   CHECK(other_array == array);
}

}
