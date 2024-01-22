#include "pch.h"

#include "string.hpp"

using namespace std::literals;

namespace we::tests {

namespace {

template<int size>
bool data_equals(const string& str, const char (&right)[size])
{
   for (int i = 0; i < size; ++i) {
      if (str.data()[i] != right[i]) return false;
   }

   return true;
}

const char long_string[] =
   "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
   "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
   "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
   "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
   "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat "
   "cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id "
   "est laborum.";

}

TEST_CASE("string default construct", "[String]")
{
   string str;

   CHECK(str.size() == 0);
   CHECK(str.data());
   CHECK(str.data()[0] == '\0');
}

TEST_CASE("string construct null terminated (short)", "[String]")
{
   string str{"Hello"};

   REQUIRE(str.size() == 5);
   REQUIRE(str.data());

   CHECK(data_equals(str, "Hello"));
}

TEST_CASE("string construct null terminated (long)", "[String]")
{
   string str{long_string};

   REQUIRE(str.size() == sizeof(long_string) - 1);
   REQUIRE(str.data());

   CHECK(data_equals(str, long_string));
}

TEST_CASE("string construct copy (short)", "[String]")
{
   string str{"Hello"};
   string str_copy{str};

   REQUIRE(str_copy.size() == 5);
   REQUIRE(str_copy.data());

   CHECK(data_equals(str_copy, "Hello"));
}

TEST_CASE("string construct copy (long)", "[String]")
{
   string str{long_string};
   string str_copy{str};

   REQUIRE(str_copy.size() == sizeof(long_string) - 1);
   REQUIRE(str_copy.data());

   CHECK(data_equals(str_copy, long_string));
}

TEST_CASE("string construct move (short)", "[String]")
{
   string str{"Hello"};
   string str_new{std::move(str)};

   REQUIRE(str.size() == 0);
   REQUIRE(str.data());

   REQUIRE(str_new.size() == 5);
   REQUIRE(str_new.data());

   CHECK(data_equals(str, ""));
   CHECK(data_equals(str_new, "Hello"));
}

TEST_CASE("string construct move (long)", "[String]")
{
   string str{long_string};
   string str_new{std::move(str)};

   REQUIRE(str.size() == 0);
   REQUIRE(str.data());

   REQUIRE(str_new.size() == sizeof(long_string) - 1);
   REQUIRE(str_new.data());

   CHECK(data_equals(str, ""));
   CHECK(data_equals(str_new, long_string));
}

TEST_CASE("string construct pointer size (short)", "[String]")
{
   string str{"Hello World", 5};

   REQUIRE(str.size() == 5);
   REQUIRE(str.data());

   CHECK(data_equals(str, "Hello"));
}

TEST_CASE("string construct pointer size (long)", "[String]")
{
   string str{long_string, sizeof(long_string) - 1};

   REQUIRE(str.size() == sizeof(long_string) - 1);
   REQUIRE(str.data());

   CHECK(data_equals(str, long_string));
}

TEST_CASE("string copy assignment short to short", "[String]")
{
   string str{"Hello"};
   string str_copy{"Goodbye"};

   str_copy = str;

   REQUIRE(str_copy.size() == 5);
   REQUIRE(str_copy.data());

   CHECK(data_equals(str_copy, "Hello"));
}

TEST_CASE("string copy assignment long to short", "[String]")
{
   string str{long_string};
   string str_copy{"Goodbye"};

   str_copy = str;

   REQUIRE(str_copy.size() == sizeof(long_string) - 1);
   REQUIRE(str_copy.data());

   CHECK(data_equals(str_copy, long_string));
}

TEST_CASE("string copy assignment short to long", "[String]")
{
   string str{"Hello"};
   string str_copy{long_string};

   str_copy = str;

   REQUIRE(str_copy.size() == 5);
   REQUIRE(str_copy.data());

   // The assignment shouldn't have reallocated memory.
   REQUIRE(str_copy.capacity() >= sizeof(long_string) - 1);

   CHECK(data_equals(str_copy, "Hello"));
}

TEST_CASE("string copy assignment long to long fitting", "[String]")
{
   string str{"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
              "eiusmod"};
   string str_copy{long_string};

   str_copy = str;

   REQUIRE(str_copy.size() == 71);
   REQUIRE(str_copy.data());

   // The assignment shouldn't have reallocated memory.
   REQUIRE(str_copy.capacity() >= sizeof(long_string) - 1);

   CHECK(data_equals(str_copy, "Lorem ipsum dolor sit amet, consectetur "
                               "adipiscing elit, sed do eiusmod"));
}

TEST_CASE("string copy assignment long to long reallocated", "[String]")
{
   string str{long_string};
   string str_copy{"Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
                   "sed do eiusmod"};

   str_copy = str;

   REQUIRE(str_copy.size() == sizeof(long_string) - 1);
   REQUIRE(str_copy.data());

   CHECK(data_equals(str_copy, long_string));
}

TEST_CASE("string copy assignment self", "[String]")
{
   string str{long_string};

   str = str;

   REQUIRE(str.size() == sizeof(long_string) - 1);
   REQUIRE(str.data());

   CHECK(data_equals(str, long_string));
}

TEST_CASE("string move assignment (short)", "[String]")
{
   string str{"Hello"};
   string str_new{"Replace Me"};

   str_new = std::move(str);

   REQUIRE(str.size() == 0);
   REQUIRE(str.data());

   REQUIRE(str_new.size() == 5);
   REQUIRE(str_new.data());

   CHECK(data_equals(str, ""));
   CHECK(data_equals(str_new, "Hello"));
}

TEST_CASE("string move assignment (long)", "[String]")
{
   string str{long_string};
   string str_new{"Replace me with another quite long string!"};

   str_new = std::move(str);

   REQUIRE(str.size() == 0);
   REQUIRE(str.data());

   REQUIRE(str_new.size() == sizeof(long_string) - 1);
   REQUIRE(str_new.data());

   CHECK(data_equals(str, ""));
   CHECK(data_equals(str_new, long_string));
}

TEST_CASE("string move assignment self", "[String]")
{
   string str{long_string};

   str = std::move(str);

   REQUIRE(str.size() == sizeof(long_string) - 1);
   REQUIRE(str.data());

   CHECK(data_equals(str, long_string));
}

TEST_CASE("string cstring assignment short to short", "[String]")
{
   string str{"Goodbye"};

   str = "Hello";

   REQUIRE(str.size() == 5);
   REQUIRE(str.data());

   CHECK(data_equals(str, "Hello"));
}

TEST_CASE("string cstring assignment long to short", "[String]")
{
   string str{"Goodbye"};

   str = long_string;

   REQUIRE(str.size() == sizeof(long_string) - 1);
   REQUIRE(str.data());

   CHECK(data_equals(str, long_string));
}

TEST_CASE("string cstring assignment short to long", "[String]")
{
   string str{long_string};

   str = "Hello";

   REQUIRE(str.size() == 5);
   REQUIRE(str.data());

   // The assignment shouldn't have reallocated memory.
   REQUIRE(str.capacity() >= sizeof(long_string) - 1);

   CHECK(data_equals(str, "Hello"));
}

TEST_CASE("string cstring assignment long to long fitting", "[String]")
{
   string str{long_string};

   str =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod";

   REQUIRE(str.size() == 71);
   REQUIRE(str.data());

   // The assignment shouldn't have reallocated memory.
   REQUIRE(str.capacity() >= sizeof(long_string) - 1);

   CHECK(data_equals(str, "Lorem ipsum dolor sit amet, consectetur "
                          "adipiscing elit, sed do eiusmod"));
}

TEST_CASE("string cstring assignment long to long reallocated", "[String]")
{
   string str{"Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
              "sed do eiusmod"};

   str = long_string;

   REQUIRE(str.size() == sizeof(long_string) - 1);
   REQUIRE(str.data());

   CHECK(data_equals(str, long_string));
}

TEST_CASE("string begin/end")
{
   string str{"Hello"};

   const auto check_iter_funcs = [&]<typename T>(T begin, T end) {
      REQUIRE(begin);
      REQUIRE(end == begin + str.size());

      const char test_string[] = "Hello";

      for (std::size_t i = 0; i < str.size(); ++i) {
         CHECK(*(begin + i) == test_string[i]);
      }
   };

   check_iter_funcs(str.begin(), str.end());
   check_iter_funcs(std::as_const(str).begin(), std::as_const(str).end());
   check_iter_funcs(str.cbegin(), str.cend());
}

TEST_CASE("string resize up short to short", "[String]")
{
   string str{"Hello"};

   str.resize(8);

   REQUIRE(str.size() == 8);
   REQUIRE(str.capacity() >= 8);

   CHECK(data_equals(str, "Hello\0\0\0"));
}

TEST_CASE("string resize up short to long", "[String]")
{
   string str{"Hello"};

   str.resize(255);

   REQUIRE(str.size() == 255);
   REQUIRE(str.capacity() >= 255);

   const char test_string[256] = "Hello";

   CHECK(data_equals(str, test_string));
}

TEST_CASE("string resize up long", "[String]")
{
   string str{long_string};

   str.resize(512);

   REQUIRE(str.size() == 512);
   REQUIRE(str.capacity() >= 512);

   char test_string[512] = {'\0'};

   std::memcpy(test_string, long_string, sizeof(long_string));

   CHECK(data_equals(str, test_string));
}

TEST_CASE("string resize down short", "[String]")
{
   string str{"Hello"};

   str.resize(2);

   REQUIRE(str.size() == 2);
   REQUIRE(str.capacity() >= 5);

   CHECK(data_equals(str, "He"));
}

TEST_CASE("string resize down long", "[String]")
{
   string str{long_string};

   str.resize(11);

   REQUIRE(str.size() == 11);
   REQUIRE(str.capacity() >= sizeof(long_string) - 1);

   CHECK(data_equals(str, "Lorem ipsum"));
}

TEST_CASE("string resize down up long", "[String]")
{
   string str{long_string};

   const std::size_t start_capcity = str.capacity();

   str.resize(11);
   str.resize(255);

   REQUIRE(str.size() == 255);
   REQUIRE(str.capacity() == start_capcity);

   const char test_string[256] = "Lorem ipsum";

   CHECK(data_equals(str, test_string));
}

TEST_CASE("string capacity (short)", "[String]")
{
   string str{"Hello"};

   REQUIRE(str.capacity() >= 5);
}

TEST_CASE("string capacity (long)", "[String]")
{
   string str{long_string};

   REQUIRE(str.capacity() >= sizeof(long_string) - 1);
}

TEST_CASE("string reserve short", "[String]")
{
   string str{"Hello"};

   str.reserve(8);

   REQUIRE(str.capacity() >= 8);
   REQUIRE(str.size() == 5);

   CHECK(data_equals(str, "Hello"));
}

TEST_CASE("string reserve short to long", "[String]")
{
   string str{"Hello"};

   str.reserve(512);

   REQUIRE(str.capacity() >= 512);
   REQUIRE(str.size() == 5);

   CHECK(data_equals(str, "Hello"));
}

TEST_CASE("string reserve long", "[String]")
{
   string str{long_string};

   str.reserve(512);

   REQUIRE(str.capacity() >= 512);
   REQUIRE(str.size() == sizeof(long_string) - 1);

   CHECK(data_equals(str, long_string));
}

TEST_CASE("string clear", "[String]")
{
   string str{"Hello"};

   const std::size_t start_capacity = str.capacity();

   str.clear();

   REQUIRE(str.capacity() == start_capacity);
   REQUIRE(str.size() == 0);

   CHECK(data_equals(str, ""));
}

TEST_CASE("string empty", "[String]")
{
   CHECK(string{}.empty());
   CHECK(not string{"Hello"}.empty());
}

TEST_CASE("string operator[]", "[String]")
{
   string str{"Hello"};

   for (std::size_t i = 0; i < str.size(); ++i) {
      CHECK(str[i] == str.data()[i]);
   }

   for (std::size_t i = 0; i < str.size(); ++i) {
      CHECK(std::as_const(str)[i] == std::as_const(str).data()[i]);
   }
}

TEST_CASE("string c_str", "[String]")
{
   string str{"Hello"};

   CHECK(str.c_str() == str.data());
   CHECK(str.c_str()[str.size()] == '\0');
}

TEST_CASE("string swap small-small", "[String]")
{
   string str_a{"Hello"};
   string str_b{"Goodbye"};

   str_a.swap(str_b);

   REQUIRE(str_a.size() == 7);
   REQUIRE(str_b.size() == 5);

   CHECK(data_equals(str_a, "Goodbye"));
   CHECK(data_equals(str_b, "Hello"));
}

TEST_CASE("string swap small-long", "[String]")
{
   string str_a{"Hello"};
   string str_b{long_string};

   char* const str_b_data = str_b.data();
   const std::size_t str_b_capacity = str_b.capacity();

   str_a.swap(str_b);

   CHECK(str_a.data() == str_b_data);
   CHECK(str_a.capacity() == str_b_capacity);

   REQUIRE(str_a.size() == sizeof(long_string) - 1);
   REQUIRE(str_b.size() == 5);

   CHECK(data_equals(str_a, long_string));
   CHECK(data_equals(str_b, "Hello"));
}

TEST_CASE("string swap long-small", "[String]")
{
   string str_a{long_string};
   string str_b{"Hello"};

   char* const str_a_data = str_a.data();
   const std::size_t str_a_capacity = str_a.capacity();

   str_a.swap(str_b);

   CHECK(str_b.data() == str_a_data);
   CHECK(str_b.capacity() == str_a_capacity);

   REQUIRE(str_a.size() == 5);
   REQUIRE(str_b.size() == sizeof(long_string) - 1);

   CHECK(data_equals(str_a, "Hello"));
   CHECK(data_equals(str_b, long_string));
}

TEST_CASE("string swap long-long", "[String]")
{
   string str_a{long_string};
   string str_b{"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed "
                "do eiusmod"};

   char* const str_a_data = str_a.data();
   const std::size_t str_a_capacity = str_a.capacity();

   char* const str_b_data = str_b.data();
   const std::size_t str_b_capacity = str_b.capacity();

   str_a.swap(str_b);

   CHECK(str_a.data() == str_b_data);
   CHECK(str_a.capacity() == str_b_capacity);
   CHECK(str_b.data() == str_a_data);
   CHECK(str_b.capacity() == str_a_capacity);

   REQUIRE(str_a.size() == 71);
   REQUIRE(str_b.size() == sizeof(long_string) - 1);

   CHECK(data_equals(str_a, "Lorem ipsum dolor sit amet, consectetur "
                            "adipiscing elit, sed do eiusmod"));
   CHECK(data_equals(str_b, long_string));
}

}
