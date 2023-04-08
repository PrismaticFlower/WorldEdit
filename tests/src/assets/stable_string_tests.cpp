#include "pch.h"

#include "assets/asset_stable_string.hpp"

using namespace std::literals;

namespace we::assets::tests {

TEST_CASE("asset stable_string", "[Assets]")
{
   stable_string empty;

   CHECK(empty == ""sv);
   CHECK(empty == "");

   stable_string another_string = stable_string{"Not Empty"};

   CHECK(another_string == "Not Empty");

   stable_string moving_is_caring = std::move(another_string);

   CHECK(another_string == "");
   CHECK(moving_is_caring == "Not Empty");
}

}
