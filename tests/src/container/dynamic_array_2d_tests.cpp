#include "pch.h"

#include "container/dynamic_array_2d.hpp"

#include <array>

namespace we::container::tests {

namespace {

template<typename T, std::size_t width>
auto make_test_array(std::initializer_list<std::array<T, width>> initializer) noexcept
{
   dynamic_array_2d<T> out{width, initializer.size()};

   const auto src_rows = initializer.begin();

   for (std::ptrdiff_t y = 0; y < out.s_height(); ++y) {
      for (std::ptrdiff_t x = 0; x < out.s_width(); ++x) {
         out[{x, y}] = src_rows[y][x];
      }
   }

   return out;
}

}

TEST_CASE("dynamic array 2d", "[Container][DynamicArray2D]")
{
   dynamic_array_2d<int> array{2, 4};

   REQUIRE(array.size() == 8);
   REQUIRE(array.ssize() == 8);
   REQUIRE(array.width() == 2);
   REQUIRE(array.height() == 4);
   REQUIRE(array.s_width() == 2);
   REQUIRE(array.s_height() == 4);
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

   std::initializer_list<int[3]> v = {{0, 1, 2}};

   REQUIRE(array == make_test_array<int, 2>({{23, 0}, {60, 0}, {0, 0}, {0, 0}}));
   REQUIRE(array != make_test_array<int, 4>({{24, 0, 60, 0}, {0, 0, 0, 0}}));

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
