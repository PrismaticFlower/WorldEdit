#include "pch.h"

#include "assets/option_file.hpp"

using namespace std::literals;

namespace we::assets::tests {

TEST_CASE("option file parse", "[Assets][OptionFile]")
{
   auto opts = parse_options(
      "-vertexlighting -keep hp_light0 hp_light1 -keep hp_fire \"hp base\" hp_smoke"sv);

   REQUIRE(opts.size() == 3);

   CHECK(opts[0].name == "-vertexlighting"sv);
   CHECK(opts[0].arguments.empty());

   CHECK(opts[1].name == "-keep"sv);
   CHECK(opts[1].arguments.size() == 2);
   CHECK(opts[1].arguments[0] == "hp_light0"sv);
   CHECK(opts[1].arguments[1] == "hp_light1"sv);

   CHECK(opts[2].name == "-keep"sv);
   CHECK(opts[2].arguments.size() == 3);
   CHECK(opts[2].arguments[0] == "hp_fire"sv);
   CHECK(opts[2].arguments[1] == "hp base"sv);
   CHECK(opts[2].arguments[2] == "hp_smoke"sv);
}

TEST_CASE("option file multiline parse", "[Assets][OptionFile]")
{
   auto opts = parse_options(
      R"(-vertexlighting 
         -keep hp_light0 
          hp_light1 
         -keep hp_fire "hp base" hp_smoke -keep hp_another_name)"sv);

   REQUIRE(opts.size() == 4);

   CHECK(opts[0].name == "-vertexlighting"sv);
   CHECK(opts[0].arguments.empty());

   CHECK(opts[1].name == "-keep"sv);
   REQUIRE(opts[1].arguments.size() == 2);
   CHECK(opts[1].arguments[0] == "hp_light0"sv);
   CHECK(opts[1].arguments[1] == "hp_light1"sv);

   CHECK(opts[2].name == "-keep"sv);
   REQUIRE(opts[2].arguments.size() == 3);
   CHECK(opts[2].arguments[0] == "hp_fire"sv);
   CHECK(opts[2].arguments[1] == "hp base"sv);
   CHECK(opts[2].arguments[2] == "hp_smoke"sv);

   CHECK(opts[3].name == "-keep"sv);
   REQUIRE(opts[3].arguments.size() == 1);
   CHECK(opts[3].arguments[0] == "hp_another_name"sv);
}

}
