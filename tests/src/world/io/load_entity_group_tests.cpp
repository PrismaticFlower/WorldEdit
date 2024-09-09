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

TEST_CASE("world entity group loading (lights)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_lights.eng", out);

   REQUIRE(group.lights.size() == 5);

   // Light 2
   {
      CHECK(group.lights[0].name == "Light 2"sv);
      CHECK(group.lights[0].position == float3{-128.463806f, 0.855094f, 22.575970f});
      CHECK(group.lights[0].rotation ==
            quaternion{0.000000f, 0.054843f, 0.998519f, 0.000000f});
      CHECK(group.lights[0].layer == 0);
      CHECK(group.lights[0].light_type == light_type::point);
      CHECK(group.lights[0].color == float3{0.501961f, 0.376471f, 0.376471f});
      CHECK(group.lights[0].static_);
      CHECK(group.lights[0].specular_caster);
      CHECK(not group.lights[0].shadow_caster);
      CHECK(group.lights[0].range == 5.0f);
      CHECK(group.lights[0].texture.empty());
   }

   // sun
   {

      CHECK(group.lights[1].name == "sun"sv);
      CHECK(group.lights[1].position == float3{-159.264923f, 19.331013f, 66.727310f});
      CHECK(group.lights[1].rotation ==
            quaternion{-0.039542f, 0.008615f, 0.922373f, -0.384204f});
      CHECK(group.lights[1].layer == 0);
      CHECK(group.lights[1].light_type == light_type::directional);
      CHECK(group.lights[1].color == float3{1.000000f, 0.882353f, 0.752941f});
      CHECK(group.lights[1].static_);
      CHECK(group.lights[1].specular_caster);
      CHECK(group.lights[1].shadow_caster);
      CHECK(group.lights[1].texture.empty());
      CHECK(group.lights[1].directional_texture_tiling == float2{1.0f, 1.0f});
      CHECK(group.lights[1].directional_texture_offset == float2{0.0f, 0.0f});
      CHECK(group.lights[1].region_name.empty());
   }

   // Light 3
   {
      CHECK(group.lights[2].name == "Light 3"sv);
      CHECK(group.lights[2].position == float3{-149.102463f, 0.469788f, -22.194153f});
      CHECK(group.lights[2].rotation ==
            quaternion{0.000000f, 0.000000f, 1.000000f, 0.000000f});
      CHECK(group.lights[2].layer == 0);
      CHECK(group.lights[2].light_type == light_type::spot);
      CHECK(group.lights[2].color == float3{1.000000, 1.000000, 1.000000});
      CHECK(group.lights[2].static_);
      CHECK(not group.lights[2].specular_caster);
      CHECK(group.lights[2].shadow_caster);
      CHECK(group.lights[2].range == 5.0f);
      CHECK(group.lights[2].inner_cone_angle == 0.785398f);
      CHECK(group.lights[2].outer_cone_angle == 0.872665f);
      CHECK(group.lights[2].texture.empty());
      CHECK(group.lights[2].bidirectional);
   }

   // Light 1
   {
      CHECK(group.lights[3].name == "Light 1"sv);
      CHECK(group.lights[3].position == float3{-129.618546f, 5.019108f, 27.300539f});
      CHECK(group.lights[3].rotation ==
            quaternion{0.380202f, 0.000000f, 0.924904f, 0.000000f});
      CHECK(group.lights[3].layer == 0);
      CHECK(group.lights[3].light_type == light_type::point);
      CHECK(group.lights[3].color == float3{0.498039f, 0.498039f, 0.627451f});
      CHECK(group.lights[3].static_);
      CHECK(group.lights[3].specular_caster);
      CHECK(not group.lights[3].shadow_caster);
      CHECK(group.lights[3].range == 16.0f);
      CHECK(group.lights[3].texture.empty());
   }

   // Light 4
   {
      CHECK(group.lights[4].name == "Light 4"sv);
      CHECK(group.lights[4].position == float3{-216.604019f, 2.231649f, 18.720726f});
      CHECK(group.lights[4].rotation ==
            quaternion{0.435918f, 0.610941f, 0.581487f, -0.314004f});
      CHECK(group.lights[4].layer == 0);
      CHECK(group.lights[4].light_type == light_type::directional_region_sphere);
      CHECK(group.lights[4].color == float3{1.000000f, 0.501961f, 0.501961f});
      CHECK(group.lights[4].static_);
      CHECK(not group.lights[4].specular_caster);
      CHECK(not group.lights[4].shadow_caster);
      CHECK(group.lights[4].texture.empty());
      CHECK(group.lights[4].directional_texture_tiling == float2{1.0f, 1.0f});
      CHECK(group.lights[4].directional_texture_offset == float2{0.0f, 0.0f});
      CHECK(group.lights[4].region_name == "lightregion1");
      CHECK(group.lights[4].ps2_blend_mode == ps2_blend_mode::blend);
      CHECK(group.lights[4].region_rotation ==
            quaternion{0.000f, 0.000f, 1.000f, 0.000f});
      CHECK(group.lights[4].region_size == float3{4.591324f, 0.100000f, 1.277475f});
   }
}
}