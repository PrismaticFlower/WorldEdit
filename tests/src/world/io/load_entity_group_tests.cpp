#include "pch.h"

#include "world/io/load_entity_group.hpp"

using namespace std::literals;

namespace we::world::tests {

TEST_CASE("world entity group loading (objects)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_objects.eng", out);

   REQUIRE(group.objects.size() == 2);

   // com_item_healthrecharge
   {
      CHECK(group.objects[0].name == "com_item_healthrecharge"sv);
      CHECK(group.objects[0].class_name == "com_item_healthrecharge"sv);
      CHECK(group.objects[0].position == float3{-32.000f, 0.008f, 32.000f});
      CHECK(group.objects[0].rotation == quaternion{0.000f, 0.000f, 1.000f, 0.000f});
      CHECK(group.objects[0].team == 0);
      CHECK(group.objects[0].layer == 0);

      REQUIRE(group.objects[0].instance_properties.size() == 2);
      CHECK(group.objects[0].instance_properties[0].key == "EffectRegion"sv);
      CHECK(group.objects[0].instance_properties[0].value == ""sv);
      CHECK(group.objects[0].instance_properties[1].key == "Radius"sv);
      CHECK(group.objects[0].instance_properties[1].value == "5.0"sv);
   }

   // com_inv_col_8
   {
      CHECK(group.objects[1].name == "com_inv_col_8"sv);
      CHECK(group.objects[1].class_name == "com_inv_col_8"sv);
      CHECK(group.objects[1].position == float3{68.000f, 0.000f, -4.000f});
      CHECK(group.objects[1].rotation == quaternion{0.000f, 0.000f, 1.000f, 0.000f});
      CHECK(group.objects[1].team == 0);
      CHECK(group.objects[1].layer == 0);
      CHECK(group.objects[1].instance_properties.size() == 0);
   }
}
}