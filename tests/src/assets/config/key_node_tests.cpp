#include "pch.h"

#include "assets/config/key_node.hpp"

#include <array>

using namespace std::literals;

namespace we::assets::config::tests {

TEST_CASE("config key node tests", "[Assets][Config]")
{
   key_node key_node{"Object"s,
                     {"lod_test0"s, "lod_test"s, 21353660},
                     {
                        {"ChildRotation"s, {1.000, 0.000, 0.000, 0.000}},
                        {"ChildPosition"s, {-256.000, 8.000, -204.000}},
                        {"SeqNo"s, {21353660}},
                        {"Team"s, {0}},
                        {"NetworkId"s, {-1}},
                        {"Property"s, {"Value"s}},
                        {"Property"s, {"Value2"s}},
                     }};

   SECTION("construction")
   {
      CHECK(key_node.key == "Object"sv);
      CHECK(key_node.values.get<std::string_view>(0) == "lod_test0"sv);
      CHECK(key_node.values.get<std::string_view>(1) == "lod_test"sv);
      CHECK(key_node.values.get<int>(2) == 21353660);
   }

   SECTION("contains")
   {
      CHECK(key_node.contains("ChildRotation"sv));
      CHECK(key_node.contains("ChildPosition"sv));
      CHECK(key_node.contains("SeqNo"sv));
      CHECK(key_node.contains("Team"sv));
      CHECK(key_node.contains("NetworkId"sv));
      CHECK(key_node.contains("Property"sv));
      CHECK(not key_node.contains("Sugar"sv));
   }

   SECTION("count")
   {
      CHECK(key_node.count("ChildRotation"sv) == 1);
      CHECK(key_node.count("ChildPosition"sv) == 1);
      CHECK(key_node.count("SeqNo"sv) == 1);
      CHECK(key_node.count("Team"sv) == 1);
      CHECK(key_node.count("NetworkId"sv) == 1);
      CHECK(key_node.count("Property"sv) == 2);
      CHECK(key_node.count("Sugar"sv) == 0);
   }

   SECTION("subscript operator")
   {
      CHECK(key_node["ChildRotation"sv].values.size() == 4);
      CHECK(key_node["Property"sv].values.get<std::string_view>(0) == "Value"sv);
      CHECK(key_node["NewProperty"sv].values.size() == 0);
   }

   SECTION("at")
   {
      CHECK(key_node.at("ChildRotation"sv).values.size() == 4);
      CHECK(key_node.at("Property"sv).values.get<std::string_view>(0) == "Value"sv);
      CHECK_THROWS(key_node.at("Meow"sv));
   }

   SECTION("find")
   {
      auto it = key_node.find("ChildRotation"sv);

      CHECK(it != key_node.end());
      CHECK(it->key == "ChildRotation"sv);
      CHECK(key_node.find("Moa"sv) == key_node.end());
   }

   SECTION("erase")
   {
      CHECK(key_node.erase("ChildRotation"sv) == 1);
      CHECK(key_node.erase("Property"sv) == 2);
      CHECK(key_node.erase("Pouakai"sv) == 0);

      CHECK_THROWS(key_node.at("ChildRotation"sv));
      CHECK_THROWS(key_node.at("Property"sv));
   }
}
}
