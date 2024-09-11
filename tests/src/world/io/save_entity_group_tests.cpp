#include "pch.h"

#include "io/read_file.hpp"
#include "world/io/save_entity_group.hpp"

using namespace std::literals;

namespace we::world::tests {

TEST_CASE("world save entity group (objects)", "[World][IO]")
{
   std::filesystem::create_directory(L"temp/world");
   const std::filesystem::path path = L"temp/world/test_objects.eng";

   const std::string_view expected_eng =
      R"(Object("com_item_healthrecharge", "com_item_healthrecharge")
{
	ChildRotation(1.000000, 0.000000, 0.000000, 0.000000);
	ChildPosition(-32.000000, 0.008000, -32.000000);
	Team(2);
	EffectRegion("");
	Radius("5.0");
}

Object("com_inv_col_8", "com_inv_col_8")
{
	ChildRotation(1.000000, 0.000000, 0.000000, 0.000000);
	ChildPosition(68.000000, 0.000000, 4.000000);
	Team(0);
}

)";

   world::entity_group group = {
      .objects =
         {
            world::object{
               .name = "com_item_healthrecharge",
               .rotation = {0.000f, -0.000f, 1.000f, -0.000f},
               .position = float3{-32.000f, 0.008f, 32.000f},
               .team = 2,
               .class_name = lowercase_string{"com_item_healthrecharge"sv},
               .instance_properties =
                  {
                     world::instance_property{.key = "EffectRegion", .value = ""},
                     world::instance_property{.key = "Radius", .value = "5.0"},
                  },
            },
            world::object{
               .name = "com_inv_col_8",
               .rotation = {0.000f, -0.000f, 1.000f, -0.000f},
               .position = float3{68.000f, 0.000f, -4.000f},
               .class_name = lowercase_string{"com_inv_col_8"sv},
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (lights)", "[World][IO]")
{
   std::filesystem::create_directory(L"temp/world");
   const std::filesystem::path path = L"temp/world/test_lights.eng";

   const std::string_view expected_eng =
      R"(Light("Light 2")
{
	Rotation(0.998519, 0.000000, 0.000000, -0.054843);
	Position(-128.463806, 0.855094, -22.575970);
	Type(2);
	Color(0.501961, 0.376471, 0.376471);
	Static();
	CastSpecular(1);
	Range(5.000000);
}

Light("sun")
{
	Rotation(0.922373, 0.384204, -0.039542, -0.008615);
	Position(-159.264923, 19.331013, -66.727310);
	Type(1);
	Color(1.000000, 0.882353, 0.752941);
	CastShadow();
	Static();
	CastSpecular(1);
	PS2BlendMode(0);
	TileUV(1.000000, 1.000000);
	OffsetUV(0.000000, 0.000000);
}

Light("Light 3")
{
	Rotation(1.000000, 0.000000, 0.000000, 0.000000);
	Position(-149.102463, 0.469788, 22.194153);
	Type(3);
	Color(1.000000, 1.000000, 1.000000);
	CastShadow();
	Static();
	Range(5.000000);
	Cone(0.785398, 0.872665);
	PS2BlendMode(0);
	Bidirectional(1);
}

Light("Light 1")
{
	Rotation(0.924904, 0.000000, 0.380202, 0.000000);
	Position(-129.618546, 5.019108, -27.300539);
	Type(2);
	Color(0.498039, 0.498039, 0.627451);
	Static();
	CastSpecular(1);
	Range(16.000000);
}

Light("Light 4")
{
	Rotation(0.581487, 0.314004, 0.435918, -0.610941);
	Position(-216.604019, 2.231649, -18.720726);
	Type(5);
	Color(1.000000, 0.501961, 0.501961);
	Static();
	PS2BlendMode(2);
	TileUV(1.000000, 1.000000);
	OffsetUV(0.000000, 0.000000);
	RegionName("lightregion1");
	RegionRotation(1.000000, 0.000000, 0.000000, 0.000000);
	RegionSize(4.591324, 0.100000, 1.277475);
}

)";

   world::entity_group group =
      {
         .lights =
            {
               world::light{
                  .name = "Light 2",
                  .rotation = quaternion{0.000000f, 0.054843f, 0.998519f, -0.000000f},
                  .position = float3{-128.463806f, 0.855094f, 22.575970f},
                  .color = float3{0.501961f, 0.376471f, 0.376471f},
                  .static_ = true,
                  .shadow_caster = false,
                  .specular_caster = true,
                  .light_type = light_type::point,
                  .range = 5.0f,
               },

               world::light{
                  .name = "sun",
                  .rotation = quaternion{-0.039542f, 0.008615f, 0.922373f, -0.384204f},
                  .position = float3{-159.264923f, 19.331013f, 66.727310f},
                  .color = float3{1.000000f, 0.882353f, 0.752941f},
                  .static_ = true,
                  .shadow_caster = true,
                  .specular_caster = true,
                  .light_type = light_type::directional,
                  .directional_texture_tiling = float2{1.0f, 1.0f},
                  .directional_texture_offset = float2{0.0f, 0.0f},
               },

               world::light{
                  .name = "Light 3",
                  .rotation = quaternion{0.000000f, -0.000000f, 1.000000f, -0.000000f},
                  .position = float3{-149.102463f, 0.469788f, -22.194153f},
                  .color = float3{1.000000, 1.000000, 1.000000},
                  .static_ = true,
                  .shadow_caster = true,
                  .specular_caster = false,
                  .light_type = light_type::spot,
                  .bidirectional = true,
                  .range = 5.0f,
                  .inner_cone_angle = 0.785398f,
                  .outer_cone_angle = 0.872665f,
               },

               world::light{
                  .name = "Light 1",
                  .rotation = quaternion{0.380202f, -0.000000f, 0.924904f, -0.000000f},
                  .position = float3{-129.618546f, 5.019108f, 27.300539f},
                  .color = float3{0.498039f, 0.498039f, 0.627451f},
                  .static_ = true,
                  .shadow_caster = false,
                  .specular_caster = true,
                  .light_type = light_type::point,
                  .range = 16.0f,
               },

               world::light{
                  .name = "Light 4",
                  .rotation = quaternion{0.435918f, 0.610941f, 0.581487f, -0.314004f},
                  .position = float3{-216.604019f, 2.231649f, 18.720726f},
                  .color = float3{1.000000f, 0.501961f, 0.501961f},
                  .static_ = true,
                  .shadow_caster = false,
                  .specular_caster = false,
                  .light_type = light_type::directional_region_sphere,
                  .ps2_blend_mode = ps2_blend_mode::blend,
                  .directional_texture_tiling = float2{1.0f, 1.0f},
                  .directional_texture_offset = float2{0.0f, 0.0f},
                  .region_name = "lightregion1",
                  .region_size = float3{4.591324f, 0.100000f, 1.277475f},
                  .region_rotation = quaternion{0.000f, -0.000f, 1.000f, -0.000f},
               },
            },
      };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}
}