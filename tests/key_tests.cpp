#include "pch.h"

#include "hotkeys.hpp"

using namespace std::literals;

namespace we::tests {

TEST_CASE("key parse_display_string", "[Hotkeys]")
{
   key key = key::void_key;
   bool ctrl = false;
   bool shift = false;

   REQUIRE(parse_display_string("Shift + 0", key, ctrl, shift));
   REQUIRE(key == key::_0);
   REQUIRE(not ctrl);
   REQUIRE(shift);
}

TEST_CASE("key parse_display_string bad", "[Hotkeys]")
{
   key key = key::void_key;
   bool ctrl = false;
   bool shift = false;

   REQUIRE(not parse_display_string("I'm Not a Valid String", key, ctrl, shift));
   REQUIRE(key == key::void_key);
   REQUIRE(not ctrl);
   REQUIRE(not shift);
}

}
