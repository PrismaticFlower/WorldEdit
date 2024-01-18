#include "pch.h"

#include "container/pinned_vector.hpp"

#include <iterator>
#include <list>
#include <string>
#include <utility>

namespace we::container::tests {

/*

Tests that still need writing!

- Test for exception during copy construct
- Test for exception during copy assign operator

- Test for exception during resize.
- Test for exception during emplace_back
- Test for exception during copy push_back
- Test for exception during copy insert
- Test for exception during copy insert count

- Test for lifetimes during emplace
- Test for lifetimes during copy insert
- Test for exception during copy insert count
- Test for lifetimes during erase

*/

TEST_CASE("pinned_vector construct empty", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536, .initial_capacity = 32}};

   REQUIRE(vec.size() == 0);
   REQUIRE(vec.capacity() >= 32);
   REQUIRE(vec.max_size() >= 65536);
}

TEST_CASE("pinned_vector construct from range", "[Container]")
{
   const std::list ints = {0, 1, 2, 3, 4, 5, 6, 7};

   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536}, ints};

   REQUIRE(vec.size() == 8);

   const int* const data = vec.data();

   for (int i = 0; i < 8; ++i) CHECK(data[i] == i);
}

TEST_CASE("pinned_vector construct from initializer_list", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   REQUIRE(vec.size() == 8);

   const int* const data = vec.data();

   for (int i = 0; i < 8; ++i) CHECK(data[i] == i);
}

TEST_CASE("pinned_vector copy construct", "[Container]")
{
   pinned_vector<int> base_vec{pinned_vector_init{.max_size = 65536},
                               std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};
   pinned_vector<int> other_vec{base_vec};

   REQUIRE(base_vec.size() == other_vec.size());
   REQUIRE(base_vec.capacity() == other_vec.capacity());
   REQUIRE(base_vec.max_size() >= other_vec.max_size());
   REQUIRE(base_vec.data() != other_vec.data());

   const int* const base_data = base_vec.data();
   const int* const other_data = other_vec.data();

   REQUIRE(base_vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(base_data[i] == other_data[i]);
}

TEST_CASE("pinned_vector move construct", "[Container]")
{
   pinned_vector<int> old_vec{pinned_vector_init{.max_size = 65536},
                              std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   const int* const old_data = old_vec.data();
   const std::size_t capacity = old_vec.capacity();

   pinned_vector<int> new_vec{std::move(old_vec)};

   REQUIRE(old_vec.size() == 0);
   REQUIRE(old_vec.capacity() == 0);
   REQUIRE(old_vec.max_size() >= 0);
   REQUIRE(old_vec.data() == nullptr);

   REQUIRE(new_vec.size() == 8);
   REQUIRE(new_vec.capacity() == capacity);
   REQUIRE(new_vec.max_size() >= 65536);
   REQUIRE(new_vec.data() == old_data);

   for (int i = 0; i < 8; ++i) CHECK(old_data[i] == i);
}

TEST_CASE("pinned_vector copy assign construct", "[Container]")
{
   pinned_vector<int> base_vec{pinned_vector_init{.max_size = 65536},
                               std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};
   pinned_vector<int> other_vec{pinned_vector_init{.max_size = 65536},
                                std::initializer_list{0, 1, 2, 3}};

   other_vec = base_vec;

   REQUIRE(base_vec.size() == other_vec.size());
   REQUIRE(base_vec.capacity() == other_vec.capacity());
   REQUIRE(base_vec.max_size() >= other_vec.max_size());
   REQUIRE(base_vec.data() != other_vec.data());

   const int* const base_data = base_vec.data();
   const int* const other_data = other_vec.data();

   REQUIRE(base_vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(base_data[i] == other_data[i]);
}

TEST_CASE("pinned_vector move assign construct", "[Container]")
{
   pinned_vector<int> old_vec{pinned_vector_init{.max_size = 65536},
                              std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};
   pinned_vector<int> new_vec{pinned_vector_init{.max_size = 65536},
                              std::initializer_list{0, 1, 2, 3}};

   const int* const old_data = old_vec.data();
   const std::size_t capacity = old_vec.capacity();

   new_vec = std::move(old_vec);

   REQUIRE(old_vec.size() == 0);
   REQUIRE(old_vec.capacity() == 0);
   REQUIRE(old_vec.max_size() >= 0);
   REQUIRE(old_vec.data() == nullptr);

   REQUIRE(new_vec.size() == 8);
   REQUIRE(new_vec.capacity() == capacity);
   REQUIRE(new_vec.max_size() >= 65536);
   REQUIRE(new_vec.data() == old_data);

   for (int i = 0; i < 8; ++i) CHECK(old_data[i] == i);
}

TEST_CASE("pinned_vector operator= initializer_list", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536}};

   vec = std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7};

   REQUIRE(vec.size() == 8);
   REQUIRE(vec.max_size() >= 65536);

   const int* const data = vec.data();

   for (int i = 0; i < 8; ++i) CHECK(data[i] == i);
}

TEST_CASE("pinned_vector assign value", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536}};

   vec.assign(64, 128);

   REQUIRE(vec.size() == 64);
   REQUIRE(vec.max_size() >= 65536);

   const int* const data = vec.data();

   for (int i = 0; i < 64; ++i) CHECK(data[i] == 128);
}

TEST_CASE("pinned_vector assign initializer_list", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536}};

   vec.assign(std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7});

   REQUIRE(vec.size() == 8);
   REQUIRE(vec.max_size() >= 65536);

   const int* const data = vec.data();

   for (int i = 0; i < 8; ++i) CHECK(data[i] == i);
}

TEST_CASE("pinned_vector empty", "[Container]")
{
   CHECK(pinned_vector<int>{pinned_vector_init{.max_size = 65536}}.empty());
   CHECK(not pinned_vector<int>{pinned_vector_init{.max_size = 65536},
                                std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}}
                .empty());
}

TEST_CASE("pinned_vector size", "[Container]")
{
   pinned_vector<int> empty_vec{pinned_vector_init{.max_size = 65536}};
   pinned_vector<int> not_empty_vec{pinned_vector_init{.max_size = 65536},
                                    std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   CHECK(empty_vec.size() == 0);
   CHECK(not_empty_vec.size() == 8);
}

TEST_CASE("pinned_vector max_size", "[Container]")
{
   pinned_vector<int> empty_vec{pinned_vector_init{.max_size = 65536}};
   pinned_vector<int> not_empty_vec{pinned_vector_init{.max_size = 65536},
                                    std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   CHECK(empty_vec.max_size() >= 65536);
   CHECK(not_empty_vec.max_size() >= 65536);
}

TEST_CASE("pinned_vector capacity", "[Container]")
{
   pinned_vector<int> empty_vec{pinned_vector_init{.max_size = 65536}};
   pinned_vector<int> not_empty_vec{pinned_vector_init{.max_size = 65536},
                                    std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   CHECK(empty_vec.capacity() >= 0);
   CHECK(not_empty_vec.capacity() >= 8);
}

TEST_CASE("pinned_vector resize up", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   vec.resize(24);

   REQUIRE(vec.size() == 24);

   const int* const data = vec.data();

   for (int i = 0; i < 8; ++i) CHECK(data[i] == i);
   for (int i = 8; i < 24; ++i) CHECK(data[i] == 0);
}

TEST_CASE("pinned_vector resize down", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   vec.resize(4);

   REQUIRE(vec.size() == 4);

   const int* const data = vec.data();

   for (int i = 0; i < 4; ++i) CHECK(data[i] == i);
}

TEST_CASE("pinned_vector resize up fill value", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   vec.resize(24, 128);

   REQUIRE(vec.size() == 24);

   const int* const data = vec.data();

   for (int i = 0; i < 8; ++i) CHECK(data[i] == i);
   for (int i = 8; i < 24; ++i) CHECK(data[i] == 128);
}

TEST_CASE("pinned_vector resize down fill value", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   vec.resize(4);

   REQUIRE(vec.size() == 4);

   const int* const data = vec.data();

   for (int i = 0; i < 4; ++i) CHECK(data[i] == i);
}

TEST_CASE("pinned_vector reserve", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   vec.reserve(20000);

   REQUIRE(vec.capacity() >= 20000);
   REQUIRE(vec.size() == 8);

   const int* const data = vec.data();

   for (int i = 0; i < 4; ++i) CHECK(data[i] == i);
}

TEST_CASE("pinned_vector reserve no op", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   vec.reserve(4);

   REQUIRE(vec.capacity() >= 4);
   REQUIRE(vec.size() == 8);

   const int* const data = vec.data();

   for (int i = 0; i < 4; ++i) CHECK(data[i] == i);
}

TEST_CASE("pinned_vector operator[]", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
   for (int i = 0; i < 8; ++i) CHECK(std::as_const(vec)[i] == i);
}

TEST_CASE("pinned_vector at", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   for (int i = 0; i < 8; ++i) CHECK(vec.at(i) == i);
   for (int i = 0; i < 8; ++i) CHECK(std::as_const(vec).at(i) == i);
}

TEST_CASE("pinned_vector at throws", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   CHECK_THROWS_AS((void)vec.at(8), std::out_of_range);
   CHECK_THROWS_AS((void)std::as_const(vec).at(8), std::out_of_range);
}

TEST_CASE("pinned_vector front", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{64, 32}};

   CHECK(vec.front() == 64);
   CHECK(std::as_const(vec).front() == 64);
}

TEST_CASE("pinned_vector back", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{64, 32}};

   CHECK(vec.back() == 32);
   CHECK(std::as_const(vec).back() == 32);
}

TEST_CASE("pinned_vector data", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   REQUIRE(vec.data() != nullptr);
   REQUIRE(std::as_const(vec).data() != nullptr);
   REQUIRE(vec.data() == std::as_const(vec).data());

   for (int i = 0; i < 4; ++i) CHECK(vec.data()[i] == i);
}

TEST_CASE("pinned_vector emplace_back", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.emplace_back(4) == 4);
   REQUIRE(vec.size() == 5);

   for (int i = 0; i < 5; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector push_back copy", "[Container]")
{
   pinned_vector<std::string> vec{pinned_vector_init{.max_size = 65536},
                                  std::initializer_list<std::string>{"Hi", "Bye", "Why"}};

   std::string new_value = "Try";

   vec.push_back(new_value);

   CHECK(new_value == "Try");

   REQUIRE(vec.size() == 4);

   CHECK(vec[0] == "Hi");
   CHECK(vec[1] == "Bye");
   CHECK(vec[2] == "Why");
   CHECK(vec[3] == "Try");
}

TEST_CASE("pinned_vector push_back move", "[Container]")
{
   pinned_vector<std::string> vec{pinned_vector_init{.max_size = 65536},
                                  std::initializer_list<std::string>{"Hi", "Bye", "Why"}};

   std::string new_value = "Try";

   vec.push_back(std::move(new_value));

   CHECK(new_value.empty());

   REQUIRE(vec.size() == 4);

   CHECK(vec[0] == "Hi");
   CHECK(vec[1] == "Bye");
   CHECK(vec[2] == "Why");
   CHECK(vec[3] == "Try");
}

TEST_CASE("pinned_vector pop_back", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2}};

   vec.pop_back();

   REQUIRE(vec.size() == 2);

   for (int i = 0; i < 2; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector emplace end", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.emplace(vec.end(), 4) == (vec.end() - 1));
   REQUIRE(vec.size() == 5);

   for (int i = 0; i < 5; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector emplace midpoint", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 2, 3}};

   CHECK(vec.emplace(vec.begin() + 1, 1) == (vec.begin() + 1));
   REQUIRE(vec.size() == 4);

   for (int i = 0; i < 4; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector emplace begin", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{1, 2, 3}};

   CHECK(vec.emplace(vec.begin(), 0) == (vec.begin()));
   REQUIRE(vec.size() == 4);

   for (int i = 0; i < 4; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector insert copy end", "[Container]")
{
   pinned_vector<std::string> vec{pinned_vector_init{.max_size = 65536},
                                  std::initializer_list<std::string>{"0", "1",
                                                                     "2", "3"}};

   std::string str = "4";

   CHECK(vec.insert(vec.end(), str) == (vec.end() - 1));
   REQUIRE(vec.size() == 5);

   CHECK(str == "4");

   CHECK(vec[0] == "0");
   CHECK(vec[1] == "1");
   CHECK(vec[2] == "2");
   CHECK(vec[3] == "3");
   CHECK(vec[4] == "4");
}

TEST_CASE("pinned_vector insert copy midpoint", "[Container]")
{
   pinned_vector<std::string> vec{pinned_vector_init{.max_size = 65536},
                                  std::initializer_list<std::string>{"0", "2", "3"}};

   std::string str = "1";

   CHECK(vec.insert(vec.begin() + 1, str) == (vec.begin() + 1));
   REQUIRE(vec.size() == 4);

   CHECK(str == "1");

   CHECK(vec[0] == "0");
   CHECK(vec[1] == "1");
   CHECK(vec[2] == "2");
   CHECK(vec[3] == "3");
}

TEST_CASE("pinned_vector insert copy begin", "[Container]")
{
   pinned_vector<std::string> vec{pinned_vector_init{.max_size = 65536},
                                  std::initializer_list<std::string>{"1", "2", "3"}};

   std::string str = "0";

   CHECK(vec.insert(vec.begin(), str) == vec.begin());
   REQUIRE(vec.size() == 4);

   CHECK(str == "0");

   CHECK(vec[0] == "0");
   CHECK(vec[1] == "1");
   CHECK(vec[2] == "2");
   CHECK(vec[3] == "3");
}

TEST_CASE("pinned_vector insert move end", "[Container]")
{
   pinned_vector<std::string> vec{pinned_vector_init{.max_size = 65536},
                                  std::initializer_list<std::string>{"0", "1",
                                                                     "2", "3"}};

   std::string str = "4";

   CHECK(vec.insert(vec.end(), std::move(str)) == (vec.end() - 1));
   REQUIRE(vec.size() == 5);

   CHECK(str.empty());

   CHECK(vec[0] == "0");
   CHECK(vec[1] == "1");
   CHECK(vec[2] == "2");
   CHECK(vec[3] == "3");
   CHECK(vec[4] == "4");
}

TEST_CASE("pinned_vector insert move midpoint", "[Container]")
{
   pinned_vector<std::string> vec{pinned_vector_init{.max_size = 65536},
                                  std::initializer_list<std::string>{"0", "2", "3"}};

   std::string str = "1";

   CHECK(vec.insert(vec.begin() + 1, std::move(str)) == (vec.begin() + 1));
   REQUIRE(vec.size() == 4);

   CHECK(str.empty());

   CHECK(vec[0] == "0");
   CHECK(vec[1] == "1");
   CHECK(vec[2] == "2");
   CHECK(vec[3] == "3");
}

TEST_CASE("pinned_vector insert move begin", "[Container]")
{
   pinned_vector<std::string> vec{pinned_vector_init{.max_size = 65536},
                                  std::initializer_list<std::string>{"1", "2", "3"}};

   std::string str = "0";

   CHECK(vec.insert(vec.begin(), std::move(str)) == vec.begin());
   REQUIRE(vec.size() == 4);

   CHECK(str.empty());

   CHECK(vec[0] == "0");
   CHECK(vec[1] == "1");
   CHECK(vec[2] == "2");
   CHECK(vec[3] == "3");
}

TEST_CASE("pinned_vector insert multiple copies end", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.insert(vec.end(), 3, 8) == (vec.begin() + 4));
   REQUIRE(vec.size() == 7);

   CHECK(vec[0] == 0);
   CHECK(vec[1] == 1);
   CHECK(vec[2] == 2);
   CHECK(vec[3] == 3);
   CHECK(vec[4] == 8);
   CHECK(vec[5] == 8);
   CHECK(vec[6] == 8);
}

TEST_CASE("pinned_vector insert multiple copies midpoint", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.insert(vec.begin() + 2, 3, 8) == (vec.begin() + 2));
   REQUIRE(vec.size() == 7);

   CHECK(vec[0] == 0);
   CHECK(vec[1] == 1);
   CHECK(vec[2] == 8);
   CHECK(vec[3] == 8);
   CHECK(vec[4] == 8);
   CHECK(vec[5] == 2);
   CHECK(vec[6] == 3);
}

TEST_CASE("pinned_vector insert multiple copies begin", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.insert(vec.begin(), 3, 8) == vec.begin());
   REQUIRE(vec.size() == 7);

   CHECK(vec[0] == 8);
   CHECK(vec[1] == 8);
   CHECK(vec[2] == 8);
   CHECK(vec[3] == 0);
   CHECK(vec[4] == 1);
   CHECK(vec[5] == 2);
   CHECK(vec[6] == 3);
}

TEST_CASE("pinned_vector insert multiple copies zero", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.insert(vec.begin(), 0, 8) == vec.begin());
   REQUIRE(vec.size() == 4);

   CHECK(vec[0] == 0);
   CHECK(vec[1] == 1);
   CHECK(vec[2] == 2);
   CHECK(vec[3] == 3);
}

TEST_CASE("pinned_vector insert initializer_list end", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.insert(vec.end(), std::initializer_list<int>{4, 5, 6, 7}) ==
         (vec.begin() + 4));
   REQUIRE(vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector insert initializer_list midpoint", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 6, 7}};

   CHECK(vec.insert(vec.begin() + 2, std::initializer_list<int>{2, 3, 4, 5}) ==
         (vec.begin() + 2));
   REQUIRE(vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector insert initializer_list begin", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{4, 5, 6, 7}};

   CHECK(vec.insert(vec.begin(), std::initializer_list<int>{0, 1, 2, 3}) ==
         vec.begin());
   REQUIRE(vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector erase end", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.erase(vec.begin() + 3) == vec.end());
   REQUIRE(vec.size() == 3);

   CHECK(vec[0] == 0);
   CHECK(vec[1] == 1);
   CHECK(vec[2] == 2);
}

TEST_CASE("pinned_vector erase midpoint", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.erase(vec.begin() + 2) == (vec.begin() + 2));
   REQUIRE(vec.size() == 3);

   CHECK(vec[0] == 0);
   CHECK(vec[1] == 1);
   CHECK(vec[2] == 3);
}

TEST_CASE("pinned_vector erase begin", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.erase(vec.begin()) == vec.begin());
   REQUIRE(vec.size() == 2);

   CHECK(vec[0] == 1);
   CHECK(vec[1] == 2);
   CHECK(vec[2] == 3);
}

TEST_CASE("pinned_vector erase iter range end", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.erase(vec.begin() + 2, vec.end()) == vec.end());
   REQUIRE(vec.size() == 2);

   CHECK(vec[0] == 0);
   CHECK(vec[1] == 1);
}

TEST_CASE("pinned_vector erase iter range midpoint", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.erase(vec.begin() + 1, vec.begin() + 3) == (vec.begin() + 1));
   REQUIRE(vec.size() == 2);

   CHECK(vec[0] == 0);
   CHECK(vec[1] == 3);
}

TEST_CASE("pinned_vector erase iter range begin", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   CHECK(vec.erase(vec.begin(), vec.begin() + 2) == vec.begin());
   REQUIRE(vec.size() == 2);

   CHECK(vec[0] == 2);
   CHECK(vec[1] == 3);
}

TEST_CASE("pinned_vector assign_range", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536}};

   const std::list ints = {0, 1, 2, 3, 4, 5, 6, 7};

   vec.assign_range(ints);

   REQUIRE(vec.size() == 8);
   REQUIRE(vec.max_size() >= 65536);

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector append_range", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   const std::list ints = {4, 5, 6, 7};

   vec.append_range(ints);

   REQUIRE(vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector insert_range end", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 2, 3}};

   const std::list ints = {4, 5, 6, 7};

   CHECK(vec.insert_range(vec.end(), ints) == (vec.begin() + 4));
   REQUIRE(vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector insert_range midpoint", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{0, 1, 6, 7}};

   const std::list ints = {2, 3, 4, 5};

   CHECK(vec.insert_range(vec.begin() + 2, ints) == (vec.begin() + 2));
   REQUIRE(vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector insert_range begin", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list<int>{4, 5, 6, 7}};

   const std::list ints = {0, 1, 2, 3};

   CHECK(vec.insert_range(vec.begin(), ints) == vec.begin());
   REQUIRE(vec.size() == 8);

   for (int i = 0; i < 8; ++i) CHECK(vec[i] == i);
}

TEST_CASE("pinned_vector swap", "[Container]")
{
   pinned_vector<int> vec_a{pinned_vector_init{.max_size = 65536},
                            std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   const int* const a_original_data = vec_a.data();
   const std::size_t a_original_capacity = vec_a.capacity();

   pinned_vector<int> vec_b{pinned_vector_init{.max_size = 256},
                            std::initializer_list{10, 11, 12, 13, 14, 15}};

   const int* const b_original_data = vec_b.data();
   const std::size_t b_original_capacity = vec_b.capacity();

   vec_a.swap(vec_b);

   REQUIRE(vec_a.data() == b_original_data);
   REQUIRE(vec_a.capacity() == b_original_capacity);
   REQUIRE(vec_a.size() == 6);
   REQUIRE(vec_a.max_size() >= 256);

   REQUIRE(vec_b.data() == a_original_data);
   REQUIRE(vec_b.capacity() == a_original_capacity);
   REQUIRE(vec_b.size() == 8);
   REQUIRE(vec_b.max_size() >= 65536);

   for (int i = 0; i < 6; ++i) CHECK(vec_a[i] == (i + 10));
   for (int i = 0; i < 8; ++i) CHECK(vec_b[i] == i);
}

TEST_CASE("pinned_vector clear", "[Container]")
{
   pinned_vector<int> vec{pinned_vector_init{.max_size = 65536},
                          std::initializer_list{0, 1, 2, 3, 4, 5, 6, 7}};

   vec.clear();

   REQUIRE(vec.size() == 0);
   REQUIRE(vec.empty());
}

}