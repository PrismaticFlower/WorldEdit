#include "pch.h"

#include "container/dynamic_array_2d.hpp"

namespace we::container::tests {

TEST_CASE("dynamic array 2d", "[Container][DynamicArray2D]")
{
   dynamic_array_2d<int> array{2, 4};

   REQUIRE(array.size() == 8);
   REQUIRE(array.ssize() == 8);
   REQUIRE(array.shape() == std::array<std::size_t, 2>{2, 4});
   REQUIRE(array.sshape() == std::array<std::ptrdiff_t, 2>{2, 4});
   REQUIRE(not array.empty());

   array[{0, 0}] = 23;
   array[{0, 1}] = 60;

   REQUIRE(array.at({0, 0}) == 23);
   REQUIRE(array.at({0, 1}) == 60);
   REQUIRE_THROWS(array.at({2, 0}));
   REQUIRE_THROWS(array.at({0, 4}));

   REQUIRE(*array.begin() == 23);
   REQUIRE(*(array.begin() + 2) == 60);
   REQUIRE(*array.cbegin() == 23);
   REQUIRE(*(array.cbegin() + 2) == 60);

   REQUIRE(array == dynamic_array_2d<int>{{std::array{23, 0}, std::array{60, 0},
                                           std::array{0, 0}, std::array{0, 0}}});
   REQUIRE(array != dynamic_array_2d<int>{
                       {std::array{24, 0, 60, 0}, std::array{0, 0, 0, 0}}});

   // test copy construction
   auto array_copy{array};

   REQUIRE(array_copy == array);

   // test move assignment
   array_copy = std::move(array);

   REQUIRE(array.size() == 0);
   REQUIRE(array != array_copy);

   // test copy assignment
   array = array_copy;

   // test swap
   array_copy[{0, 0}] = 1;

   swap(array, array_copy);

   REQUIRE(array[{0, 0}] == 1);
   REQUIRE(array_copy[{0, 0}] == 23);

   array.swap(array_copy);

   REQUIRE(array[{0, 0}] == 23);
   REQUIRE(array_copy[{0, 0}] == 1);

   // test move assignment
   array = std::move(array_copy);

   REQUIRE(array[{0, 0}] == 1);
   REQUIRE(array_copy.empty());
}

}
