#include "pch.h"

#include "assets/req/io.hpp"

#include <string_view>

using namespace std::literals;

namespace we::assets::req::tests {

namespace {

const std::string_view valid_req_test = R"(// Such a nice comment.

ucft
{
   REQN // OOo comments
   {
      "script"
      "first"
      // "half"
      "second"
   }

   // More comments!

   REQN
   {
      "texture"
      "alignment=2048"
      "first"
      "second"
   }

   REQN
   {
      "model"
      "platform=xbox"
      "first"
      "second"
   }
    
   REQN
   {
      "envfx"
      "platform=ps2"
      "alignment=8192"
      "first"
      "second"
   }

   REQN // This gets dropped during parse since it is useless.
   {
   }

   REQN
   {
      "terrain"
   }

   REQN
   {
      "world"
      "platform=pc"
   }
}




)";

const std::string_view invalid_req_test = R"(ucft
{
   REQN // OOo comments
   {
      "script"
   }
// Missing close bracket :(
)";

const std::string_view invalid_req_unknown_platform_test = R"(ucft
{
   REQN // OOo comments
   {
      "script"
      "platform=ps5"
   }
}
)";

}

TEST_CASE(".req reading", "[Assets][REQ]")
{
   auto requirements = read(valid_req_test);

   REQUIRE(requirements.size() == 6);

   CHECK(requirements[0].file_type == "script");
   CHECK(requirements[0].platform == platform::all);
   CHECK(requirements[0].alignment == 0);
   REQUIRE(requirements[0].entries.size() == 2);
   CHECK(requirements[0].entries[0] == "first");
   CHECK(requirements[0].entries[1] == "second");

   CHECK(requirements[1].file_type == "texture");
   CHECK(requirements[1].platform == platform::all);
   CHECK(requirements[1].alignment == 2048);
   REQUIRE(requirements[1].entries.size() == 2);
   CHECK(requirements[1].entries[0] == "first");
   CHECK(requirements[1].entries[1] == "second");

   CHECK(requirements[2].file_type == "model");
   CHECK(requirements[2].platform == platform::xbox);
   CHECK(requirements[2].alignment == 0);
   REQUIRE(requirements[2].entries.size() == 2);
   CHECK(requirements[2].entries[0] == "first");
   CHECK(requirements[2].entries[1] == "second");

   CHECK(requirements[3].file_type == "envfx");
   CHECK(requirements[3].platform == platform::ps2);
   CHECK(requirements[3].alignment == 8192);
   REQUIRE(requirements[3].entries.size() == 2);
   CHECK(requirements[3].entries[0] == "first");
   CHECK(requirements[3].entries[1] == "second");

   CHECK(requirements[4].file_type == "terrain");
   CHECK(requirements[4].platform == platform::all);
   CHECK(requirements[4].alignment == 0);
   REQUIRE(requirements[4].entries.size() == 0);

   CHECK(requirements[5].file_type == "world");
   CHECK(requirements[5].platform == platform::pc);
   CHECK(requirements[5].alignment == 0);
   REQUIRE(requirements[5].entries.size() == 0);
}

TEST_CASE(".req failed reading missing closing bracket", "[Assets][REQ]")
{
   REQUIRE_THROWS(read(invalid_req_test));
}

TEST_CASE(".req failed reading unknown platform", "[Assets][REQ]")
{
   REQUIRE_THROWS(read(invalid_req_unknown_platform_test));
}

}
