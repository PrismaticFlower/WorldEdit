#include "pch.h"

#include "assets/config/io.hpp"

#include <array>
#include <string_view>

using namespace std::literals;
using namespace Catch::literals;

namespace {
const auto valid_config_test = R"(
Object("lod_test1200", "lod_test", 21353660)
{
   ChildRotation(1.000, 0.000, 0.000, 0.000);
   ChildPosition(-256.000, 8.000, -204.000);
   SeqNo(21353660);
   Team(0);
   NetworkId(-1);
}

// A comment.
Nodes(2) // Another Comment
{
   Node()
   { // Yet another comment.
      Position(-131.019501, 0.000000, -251.667999);
      Knot(0.000000);
      Data(1);
      Time(1.000000);
      PauseTime(0.000000);
      Rotation(1.000000, 0.000000, 0.000000, 0.000000);
      Properties(0)
      {
      }
   }
   Node()
   {
      Position(-133.385574, 0.000000, -247.513336);
      Knot(0.000000);
      Data(1);
      Time(1.000000);
      PauseTime(0.000000);
      Rotation(1.000000, 0.000000, 0.000000, 0.000000);
      Properties(0)
      {
      }
   }
}
)"sv;
}

namespace sk::assets::config::tests {
TEST_CASE("config io valid tests", "[Assets][Config]")
{
   node config = read_config(valid_config_test);

   auto config_it = config.begin();

   REQUIRE(config_it != config.end());
   {
      REQUIRE(config_it->key == "Object"sv);
      CHECK(config_it->values.get<std::string_view>(0) == "lod_test1200"sv);
      CHECK(config_it->values.get<std::string_view>(1) == "lod_test"sv);
      CHECK(config_it->values.get<int>(2) == 21353660);

      auto object_it = config_it->begin();

      REQUIRE(object_it != config_it->end());
      {
         REQUIRE(object_it->key == "ChildRotation"sv);
         CHECK(object_it->values.get<float>(0) == 1.0_a);
         CHECK(object_it->values.get<float>(1) == 0.0_a);
         CHECK(object_it->values.get<float>(2) == 0.0_a);
         CHECK(object_it->values.get<float>(3) == 0.0_a);
      }

      ++object_it;

      REQUIRE(object_it != config_it->end());
      {
         REQUIRE(object_it->key == "ChildPosition"sv);
         CHECK(object_it->values.get<float>(0) == -256.0_a);
         CHECK(object_it->values.get<float>(1) == 8.0_a);
         CHECK(object_it->values.get<float>(2) == -204.0_a);
      }

      ++object_it;

      REQUIRE(object_it != config_it->end());
      {
         REQUIRE(object_it->key == "SeqNo"sv);
         CHECK(object_it->values.get<int>(0) == 21353660);
      }

      ++object_it;

      REQUIRE(object_it != config_it->end());
      {
         REQUIRE(object_it->key == "Team"sv);
         CHECK(object_it->values.get<int>(0) == 0);
      }

      ++object_it;

      REQUIRE(object_it != config_it->end());
      {
         REQUIRE(object_it->key == "NetworkId"sv);
         CHECK(object_it->values.get<int>(0) == -1);
      }
   }

   ++config_it;

   REQUIRE(config_it != config.end());
   {
      REQUIRE(config_it->key == "Nodes"sv);
      CHECK(config_it->values.get<int>(0) == 2);

      auto nodes_list_it = config_it->begin();

      auto node_test = [](node::iterator nodes_list_it,
                          std::array<float, 3> expected_pos) {
         REQUIRE(nodes_list_it->key == "Node"sv);
         CHECK(not nodes_list_it->empty());

         auto node_it = nodes_list_it->begin();

         REQUIRE(node_it != nodes_list_it->end());
         {
            REQUIRE(node_it->key == "Position"sv);
            CHECK(node_it->values.get<float>(0) == Approx{expected_pos[0]});
            CHECK(node_it->values.get<float>(1) == Approx{expected_pos[1]});
            CHECK(node_it->values.get<float>(2) == Approx{expected_pos[2]});
         }

         ++node_it;

         REQUIRE(node_it != nodes_list_it->end());
         {
            REQUIRE(node_it->key == "Knot"sv);
            CHECK(node_it->values.get<float>(0) == 0.0_a);
         }

         ++node_it;

         REQUIRE(node_it != nodes_list_it->end());
         {
            REQUIRE(node_it->key == "Data"sv);
            CHECK(node_it->values.get<int>(0) == 1);
         }

         ++node_it;

         REQUIRE(node_it != nodes_list_it->end());
         {
            REQUIRE(node_it->key == "Time"sv);
            CHECK(node_it->values.get<float>(0) == 1.0_a);
         }

         ++node_it;

         REQUIRE(node_it != nodes_list_it->end());
         {
            REQUIRE(node_it->key == "PauseTime"sv);
            CHECK(node_it->values.get<float>(0) == 0.0_a);
         }

         ++node_it;

         REQUIRE(node_it != nodes_list_it->end());
         {
            REQUIRE(node_it->key == "Rotation"sv);
            CHECK(node_it->values.get<float>(0) == 1.0_a);
            CHECK(node_it->values.get<float>(1) == 0.0_a);
            CHECK(node_it->values.get<float>(2) == 0.0_a);
            CHECK(node_it->values.get<float>(3) == 0.0_a);
         }

         ++node_it;

         REQUIRE(node_it != nodes_list_it->end());
         {
            REQUIRE(node_it->key == "Properties"sv);
            CHECK(node_it->values.get<int>(0) == 0);
         }

         ++node_it;
      };

      REQUIRE(nodes_list_it != config_it->end());
      {
         node_test(nodes_list_it, {-131.019501f, 0.0f, -251.667999f});
      }

      ++nodes_list_it;

      REQUIRE(nodes_list_it != config_it->end());
      {
         node_test(nodes_list_it, {-133.385574f, 0.0f, -247.513336f});
      }
   }
}

}