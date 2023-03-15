#include "pch.h"

#include "assets/odf/properties.hpp"

using namespace std::literals;

namespace we::assets::odf::tests {

TEST_CASE(".odf properties empty container", "[Assets][ODF]")
{
   properties properties;

   REQUIRE(properties.empty());
   REQUIRE(properties.size() == 0);
}

TEST_CASE(".odf properties list construction", "[Assets][ODF]")
{
   properties properties{
      {"GeometryName", "cool_model"},
      {"OtherKey", "OtherValue"},
      {"PiApprox", "3"},
   };

   SECTION("core checks")
   {
      REQUIRE(properties.size() == 3);
      REQUIRE(not properties.empty());

      properties.reserve(8);
      REQUIRE(properties.capacity() == 8);
   }

   SECTION("index value checks")
   {
      REQUIRE(properties[0].key == "GeometryName"sv);
      REQUIRE(properties[0].value == "cool_model"sv);
      REQUIRE(properties[1].key == "OtherKey"sv);
      REQUIRE(properties[1].value == "OtherValue"sv);
      REQUIRE(properties[2].key == "PiApprox"sv);
      REQUIRE(properties[2].value == "3"sv);

      REQUIRE(properties.at(0).key == "GeometryName"sv);
      REQUIRE(properties.at(0).value == "cool_model"sv);
      REQUIRE(properties.at(1).key == "OtherKey"sv);
      REQUIRE(properties.at(1).value == "OtherValue"sv);
      REQUIRE(properties.at(2).key == "PiApprox"sv);
      REQUIRE(properties.at(2).value == "3"sv);
   }

   SECTION("key value checks")
   {
      REQUIRE(properties["GeometryName"sv] == "cool_model"sv);
      REQUIRE(properties["OtherKey"sv] == "OtherValue"sv);
      REQUIRE(properties["PiApprox"sv] == "3"sv);

      // casing doesn't matter for key-value lookups.
      REQUIRE(properties["geoMetrYnamE"sv] == "cool_model"sv);
   }

   SECTION("push checks")
   {
      properties.push_back({.key = "Hello"sv, .value = "Person"sv});

      REQUIRE(properties.size() == 4);
      REQUIRE(properties[3].key == "Hello"sv);
      REQUIRE(properties[3].value == "Person"sv);
   }

   SECTION("invalid access checks")
   {
      REQUIRE_THROWS(properties[437]);
      REQUIRE_THROWS(properties["BadKey"sv]);
      REQUIRE_THROWS(properties.at(437));
      REQUIRE_THROWS(properties.at("BadKey"sv));
   }
}
}
