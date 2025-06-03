#include "pch.h"

#include "io/read_file.hpp"
#include "world/io/save_entity_group.hpp"

using namespace std::literals;

namespace we::world::tests {

TEST_CASE("world save entity group (objects)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_objects.eng";

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
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_lights.eng";

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

TEST_CASE("world save entity group (paths)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_paths.eng";

   const std::string_view expected_eng =
      R"(Path("Path 0")
{
	SplineType("Catmull-Rom");

	Properties(2)
	{
		PropKey("PropValue");
		PropEmpty("");
	}

	Nodes(3)
	{
		Node()
		{
			Position(-16.041691, 0.000000, -31.988783);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(2)
			{
				PropKey("PropValue");
				PropEmpty("");
			}
		}

		Node()
		{
			Position(-31.982189, 0.000000, -48.033310);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(-48.012756, 0.000000, -31.962399);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

	}

}

Path("type_EntityPath Path 1")
{
	SplineType("None");

	Properties(0)
	{
	}

	Nodes(1)
	{
		Node()
		{
			Position(-16.041691, 0.000000, -31.988783);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

	}

}

)";

   world::entity_group
      group =
         {
            .paths =
               {
                  world::path{
                     .name = "Path 0",
                     .type = path_type::none,
                     .spline_type = path_spline_type::catmull_rom,
                     .properties =
                        {
                           world::path::property{.key = "PropKey", .value = "PropValue"},
                           world::path::property{.key = "PropEmpty", .value = ""},
                        },
                     .nodes =
                        {
                           world::path::node{
                              .rotation = quaternion{0.0f, -0.0f, 1.0f, -0.0f},
                              .position = float3{-16.041691f, 0.000000f, 31.988783f},
                              .properties =
                                 {
                                    world::path::property{.key = "PropKey", .value = "PropValue"},
                                    world::path::property{.key = "PropEmpty", .value = ""},
                                 },
                           },

                           world::path::node{
                              .rotation = quaternion{0.0f, -0.0f, 1.0f, -0.0f},
                              .position = float3{-31.982189f, 0.000000f, 48.033310f},
                           },

                           world::path::node{
                              .rotation = quaternion{0.0f, -0.0f, 1.0f, -0.0f},
                              .position = float3{-48.012756f, 0.000000f, 31.962399f},
                           },
                        },
                  },

                  world::path{
                     .name = "Path 1",
                     .type = path_type::entity_follow,
                     .spline_type = path_spline_type::none,

                     .nodes =
                        {
                           world::path::node{
                              .rotation = quaternion{0.0f, -0.0f, 1.0f, -0.0f},
                              .position = float3{-16.041691f, 0.000000f, 31.988783f},
                           },
                        },
                  },
               },
         };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (regions)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_regions.eng";

   const std::string_view expected_eng =
      R"(Region("foleyfx water", 0)
{
	Position(-32.000000, 16.000000, -32.000000);
	Rotation(1.000000, 0.000000, 0.000000, 0.000000);
	Size(16.000000, 16.000000, 16.000000);
	Name("Region0");
}

)";

   world::entity_group group = {
      .regions =
         {
            world::region{
               .name = "Region0",
               .rotation = quaternion{0.000f, -0.000f, 1.000f, -0.000f},
               .position = float3{-32.000000f, 16.000000f, 32.000000f},
               .size = float3{16.000000f, 16.000000f, 16.000000f},
               .shape = region_shape::box,
               .description = "foleyfx water",
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (sectors)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_sectors.eng";

   const std::string_view expected_eng =
      R"(Sector("sector")
{
	Base(0.000000);
	Height(10.000000);
	Point(-157.581009, 4.900336);
	Point(-227.199097, 7.364827);
	Point(-228.642029, -40.347687);
	Point(-159.451279, -40.488800);
	Object("tat3_bldg_keeper");
}

Sector("Sector-1")
{
	Base(0.000000);
	Height(10.000000);
	Point(-196.648041, -125.908623);
	Point(-195.826218, -49.666763);
	Point(-271.034851, -48.864563);
	Point(-274.260132, -128.690567);
	Object("lod_test120");
	Object("lod_test2010");
	Object("lod_test12");
	Object("lod_test201");
}

)";

   world::entity_group group = {
      .sectors =
         {
            world::sector{
               .name = "sector",
               .base = 0.0f,
               .height = 10.0f,
               .points =
                  {
                     float2{-157.581009f, -4.900336f},
                     float2{-227.199097f, -7.364827f},
                     float2{-228.642029f, 40.347687f},
                     float2{-159.451279f, 40.488800f},
                  },
               .objects = {"tat3_bldg_keeper"},
            },

            world::sector{
               .name = "Sector-1",
               .base = 0.0f,
               .height = 10.0f,
               .points =
                  {
                     float2{-196.648041f, 125.908623f},
                     float2{-195.826218f, 49.666763f},
                     float2{-271.034851f, 48.864563f},
                     float2{-274.260132f, 128.690567f},
                  },
               .objects = {"lod_test120", "lod_test2010", "lod_test12", "lod_test201"},
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (portals)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_portals.eng";

   const std::string_view expected_eng =
      R"(Portal("Portal")
{
	Rotation(1.000000, 0.000000, 0.000000, 0.000000);
	Position(-193.661575, 2.097009, -31.728502);
	Width(2.920000);
	Height(4.120000);
	Sector1("sector");
	Sector2("Sector-1");
}

)";

   world::entity_group group = {
      .portals =
         {
            world::portal{
               .name = "Portal",
               .rotation = quaternion{0.000f, -0.000f, 1.000f, -0.000f},
               .position = float3{-193.661575f, 2.097009f, 31.728502f},
               .width = 2.92f,
               .height = 4.12f,
               .sector1 = "sector",
               .sector2 = "Sector-1",
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (hintnodes)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_hintnodes.eng";

   const std::string_view expected_eng =
      R"(Hint("HintNode0", "5")
{
	Position(-70.045296, 1.000582, -19.298828);
	Rotation(0.303753, 0.399004, -0.569245, -0.651529);
	Radius(7.692307);
	Mode(1);
	CommandPost("cp1");
}

Hint("HintNode1", "5")
{
	Position(-136.048569, 0.500000, -25.761259);
	Rotation(0.090763, 0.000000, -0.995872, 0.000000);
	PrimaryStance(7);
	Mode(3);
	CommandPost("cp2");
}

)";

   world::entity_group group = {
      .hintnodes =
         {
            world::hintnode{
               .name = "HintNode0",
               .rotation = quaternion{-0.569245f, 0.651529f, 0.303753f, -0.399004f},
               .position = float3{-70.045296f, 1.000582f, 19.298828f},
               .type = hintnode_type::mine,
               .mode = hintnode_mode::attack,
               .radius = 7.692307f,
               .primary_stance = stance_flags::none,
               .secondary_stance = stance_flags::none,
               .command_post = "cp1",
            },

            world::hintnode{
               .name = "HintNode1",
               .rotation = quaternion{-0.995872f, -0.000000f, 0.090763f, -0.000000f},
               .position = float3{-136.048569f, 0.500000f, 25.761259f},
               .type = hintnode_type::mine,
               .mode = hintnode_mode::both,
               .radius = 0.0f,
               .primary_stance =
                  (stance_flags::stand | stance_flags::crouch | stance_flags::prone),
               .secondary_stance = stance_flags::none,
               .command_post = "cp2",
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (barriers)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_barriers.eng";

   const std::string_view expected_eng =
      R"(Barrier("Barrier0")
{
	Corner(72.596146, 2.000000, -31.159695);
	Corner(86.691788, 2.000000, -0.198154);
	Corner(99.806580, 2.000000, -6.168838);
	Corner(85.710938, 2.000000, -37.130379);
	Flag(32);
}

)";

   world::entity_group group = {
      .barriers =
         {
            world::barrier{
               .name = "Barrier0",
               .position = float3{86.2013626f, 2.0f, 18.6642666f},
               .size = float2{7.20497799f, 17.0095882f},
               .rotation_angle = 2.71437049f,
               .flags = ai_path_flags::flyer,
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (planning)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_planning.eng";

   const std::string_view expected_eng =
      R"(Hub("Hub0")
{
	Pos(-63.822487, 0.000000, 9.202278);
	Radius(8.000000);
}

Hub("Hub1")
{
	Pos(-121.883095, 1.000000, 30.046543);
	Radius(7.586431);
	BranchWeight("Hub3",100.000000,"Connection0",32);
	BranchWeight("Hub3",75.000000,"Connection0",16);
	BranchWeight("Hub3",25.000000,"Connection0",8);
	BranchWeight("Hub3",7.500000,"Connection0",4);
	BranchWeight("Hub3",15.000000,"Connection0",2);
	BranchWeight("Hub3",20.000000,"Connection0",1);
}

Hub("Hub2")
{
	Pos(-54.011314, 2.000000, 194.037018);
	Radius(13.120973);
}

Hub("Hub3")
{
	Pos(-163.852570, 3.000000, 169.116760);
	Radius(12.046540);
}

Connection("Connection0")
{
	Start("Hub0");
	End("Hub1");
	Flag(63);
}

Connection("Connection1")
{
	Start("Hub3");
	End("Hub2");
	Flag(2);
}

)";

   world::entity_group group = {
      .planning_hubs =
         {
            world::planning_hub{
               .name = "Hub0",
               .position = float3{-63.822487f, 0.0f, -9.202278f},
               .radius = 8.0f,
            },

            world::planning_hub{
               .name = "Hub1",
               .position = float3{-121.883095f, 1.0f, -30.046543f},
               .radius = 7.586431f,
               .weights = {world::planning_branch_weights{
                  .hub_index = 3,
                  .connection_index = 0,
                  .soldier = 20.0f,
                  .hover = 15.0f,
                  .small = 7.5f,
                  .medium = 25.0f,
                  .huge = 75.0f,
                  .flyer = 100.0f,
               }},
            },

            world::planning_hub{
               .name = "Hub2",
               .position = float3{-54.011314f, 2.0f, -194.037018f},
               .radius = 13.120973f,
            },

            world::planning_hub{
               .name = "Hub3",
               .position = float3{-163.852570f, 3.0f, -169.116760f},
               .radius = 12.046540f,
            },
         },

      .planning_connections =
         {
            world::planning_connection{
               .name = "Connection0",
               .start_hub_index = 0,
               .end_hub_index = 1,
               .flags = (ai_path_flags::soldier | ai_path_flags::hover |
                         ai_path_flags::small | ai_path_flags::medium |
                         ai_path_flags::huge | ai_path_flags::flyer),
            },

            world::planning_connection{
               .name = "Connection1",
               .start_hub_index = 3,
               .end_hub_index = 2,
               .flags = ai_path_flags::hover,
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (boundaries)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_boundaries.eng";

   const std::string_view expected_eng =
      R"(Boundary("boundary")
{
	Node(383.557434, 1.000000, -4.797791);
	Node(332.111023, 1.000000, 187.202209);
	Node(191.557434, 1.000000, 327.755798);
	Node(-0.442566, 1.000000, 379.202209);
	Node(-192.442566, 1.000000, 327.755798);
	Node(-332.996155, 1.000000, 187.202209);
	Node(-384.442566, 1.000000, -4.797791);
	Node(-332.996155, 1.000000, -196.797791);
	Node(-192.442566, 1.000000, -337.351379);
	Node(-0.442566, 1.000000, -388.797791);
	Node(191.557434, 1.000000, -337.351379);
	Node(332.111023, 1.000000, -196.797791);
}

)";

   world::entity_group group = {
      .boundaries =
         {
            world::boundary{
               .name = "boundary",
               .position = float3{-0.442565918f, 1.0f, 4.79779053f},
               .size = float2{384.000000f, 384.000000f},
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (measurements)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_measurements.eng";

   const std::string_view expected_eng =
      R"(Measurement("Measurement0")
{
	Start(1.000000, 0.000000, 0.000000);
	End(2.000000, 0.000000, 1.000000);
}

)";

   world::entity_group group = {
      .measurements =
         {
            world::measurement{
               .start = float3{1.0f, 0.0f, -0.0f},
               .end = float3{2.0f, 0.0f, -1.0f},
               .name = "Measurement0",
            },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (blocks, boxes)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_blocks_boxes.eng";

   const std::string_view expected_eng =
      R"(BlocksBoxes(3)
{
   Box()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1, 2, 3, 4, 5);
      SurfaceTextureMode(0, 0, 0, 0, 0, 0);
      SurfaceTextureRotation(2, 2, 2, 2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 256, 256, 0, 0);
   }
   Box()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2, 2, 2, 2, 2);
      SurfaceTextureMode(1, 1, 1, 1, 5, 1);
      SurfaceTextureRotation(1, 1, 1, 1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
   Box()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0, 0, 0, 0, 0);
      SurfaceTextureMode(2, 2, 2, 2, 2, 2);
      SurfaceTextureRotation(0, 0, 0, 0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
}

)";

   world::entity_group
      group =
         {
            .blocks =
               {
                  .boxes =
                     {
                        world::block_description_box{
                           .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                           .position = {8.5f, 4.5f, 2.0f},
                           .size = {4.0f, 4.0f, 4.0f},
                           .surface_materials = {0, 1, 2, 3, 4, 5},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{-1, -2},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{256, 256},
                                 std::array<uint16, 2>{0, 0},
                              },
                        },
                        world::block_description_box{
                           .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                           .position = {10.0f, 16.0f, 12.0f},
                           .size = {8.0f, 4.0f, 8.0f},
                           .surface_materials = {2, 2, 2, 2, 2, 2},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::local_space_zy,
                                 block_texture_mode::world_space_zy,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{-2, -2},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{1024, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                              },
                        },
                        world::block_description_box{
                           .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                           .position = {6.0f, 6.0f, 6.0f},
                           .size = {5.0f, 5.0f, 5.0f},
                           .surface_materials = {0, 0, 0, 0, 0, 0},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                              },
                        },
                     },

               },
         };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (blocks, ramps)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_blocks_ramps.eng";

   const std::string_view expected_eng =
      R"(BlocksRamps(3)
{
   Ramp()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1, 2, 3, 4);
      SurfaceTextureMode(0, 0, 0, 0, 0);
      SurfaceTextureRotation(2, 2, 2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 256, 256);
   }
   Ramp()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2, 2, 2, 2);
      SurfaceTextureMode(1, 1, 1, 1, 5);
      SurfaceTextureRotation(1, 1, 1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
   Ramp()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0, 0, 0, 0);
      SurfaceTextureMode(2, 2, 2, 2, 2);
      SurfaceTextureRotation(0, 0, 0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
}

)";

   world::entity_group
      group =
         {
            .blocks =
               {
                  .ramps =
                     {
                        world::block_description_ramp{
                           .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                           .position = {8.5f, 4.5f, 2.0f},
                           .size = {4.0f, 4.0f, 4.0f},
                           .surface_materials = {0, 1, 2, 3, 4},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{-1, -2},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{256, 256},
                              },
                        },
                        world::block_description_ramp{
                           .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                           .position = {10.0f, 16.0f, 12.0f},
                           .size = {8.0f, 4.0f, 8.0f},
                           .surface_materials = {2, 2, 2, 2, 2},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::local_space_zy,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{-2, -2},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{1024, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                              },
                        },
                        world::block_description_ramp{
                           .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                           .position = {6.0f, 6.0f, 6.0f},
                           .size = {5.0f, 5.0f, 5.0f},
                           .surface_materials = {0, 0, 0, 0, 0},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                              },
                        },
                     },

               },
         };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (blocks, quads)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_blocks_quads.eng";

   const std::string_view expected_eng =
      R"(BlocksQuads(3)
{
   Quad()
   {
      Vertex0(0, 0, 0);
      Vertex1(1, 0, 0);
      Vertex2(1, 0, 1);
      Vertex3(0, 0, 1);
      QuadSplit(0);
      SurfaceMaterials(0);
      SurfaceTextureMode(0);
      SurfaceTextureRotation(2);
      SurfaceTextureScale(-1, -2);
      SurfaceTextureOffset(256, 256);
   }
   Quad()
   {
      Vertex0(0, 0, 0);
      Vertex1(1.5, 0, 0);
      Vertex2(1, 0, 1);
      Vertex3(0, 0, 1.5);
      QuadSplit(1);
      SurfaceMaterials(2);
      SurfaceTextureMode(1);
      SurfaceTextureRotation(1);
      SurfaceTextureScale(0, 0);
      SurfaceTextureOffset(1024, 0);
   }
   Quad()
   {
      Vertex0(0, 0, 0);
      Vertex1(8, 0, 0);
      Vertex2(8, 0, 8);
      Vertex3(0, 0, 8);
      QuadSplit(0);
      SurfaceMaterials(0);
      SurfaceTextureMode(2);
      SurfaceTextureRotation(0);
      SurfaceTextureScale(0, 0);
      SurfaceTextureOffset(0, 0);
   }
}

)";

   world::entity_group
      group =
         {
            .blocks =
               {
                  .quads =
                     {
                        world::block_description_quad{
                           .vertices =
                              {
                                 float3{0.0f, 0.0f, 0.0f},
                                 float3{1.0f, 0.0f, 0.0f},
                                 float3{1.0f, 0.0f, 1.0f},
                                 float3{0.0f, 0.0f, 1.0f},
                              },
                           .quad_split = world::block_quad_split::regular,
                           .surface_materials = {0},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_auto,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d180,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{-1, -2},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{256, 256},
                              },
                        },
                        world::block_description_quad{
                           .vertices =
                              {
                                 float3{0.0f, 0.0f, 0.0f},
                                 float3{1.5f, 0.0f, 0.0f},
                                 float3{1.0f, 0.0f, 1.0f},
                                 float3{0.0f, 0.0f, 1.5f},
                              },
                           .quad_split = world::block_quad_split::alternate,
                           .surface_materials = {2},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_zy,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d90,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{1024, 0},
                              },
                        },
                        world::block_description_quad{
                           .vertices =
                              {
                                 float3{0.0f, 0.0f, 0.0f},
                                 float3{8.0f, 0.0f, 0.0f},
                                 float3{8.0f, 0.0f, 8.0f},
                                 float3{0.0f, 0.0f, 8.0f},
                              },
                           .quad_split = world::block_quad_split::regular,
                           .surface_materials = {0},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_xz,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d0,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{0, 0},
                              },
                        },
                     },

               },
         };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (blocks, cylinders)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_blocks_cylinders.eng";

   const std::string_view expected_eng =
      R"(BlocksCylinders(3)
{
   Cylinder()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1, 2);
      SurfaceTextureMode(0, 0, 0);
      SurfaceTextureRotation(2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 256, 256);
   }
   Cylinder()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2, 2);
      SurfaceTextureMode(1, 1, 5);
      SurfaceTextureRotation(1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0);
   }
   Cylinder()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0, 0);
      SurfaceTextureMode(2, 2, 2);
      SurfaceTextureRotation(0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0);
   }
}

)";

   world::entity_group
      group =
         {
            .blocks =
               {
                  .cylinders =
                     {
                        world::block_description_cylinder{
                           .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                           .position = {8.5f, 4.5f, 2.0f},
                           .size = {4.0f, 4.0f, 4.0f},
                           .surface_materials = {0, 1, 2},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                                 block_texture_mode::world_space_auto,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                                 block_texture_rotation::d180,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{-1, -2},
                                 std::array<int8, 2>{0, 0},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{256, 256},
                              },
                        },
                        world::block_description_cylinder{
                           .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                           .position = {10.0f, 16.0f, 12.0f},
                           .size = {8.0f, 4.0f, 8.0f},
                           .surface_materials = {2, 2, 2},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::world_space_zy,
                                 block_texture_mode::local_space_zy,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                                 block_texture_rotation::d90,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{-2, -2},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{1024, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                              },
                        },
                        world::block_description_cylinder{
                           .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                           .position = {6.0f, 6.0f, 6.0f},
                           .size = {5.0f, 5.0f, 5.0f},
                           .surface_materials = {0, 0, 0},
                           .surface_texture_mode =
                              {
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                                 block_texture_mode::world_space_xz,
                              },
                           .surface_texture_rotation =
                              {
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                                 block_texture_rotation::d0,
                              },
                           .surface_texture_scale =
                              {
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                                 std::array<int8, 2>{0, 0},
                              },
                           .surface_texture_offset =
                              {
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                                 std::array<uint16, 2>{0, 0},
                              },
                        },
                     },

               },
         };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (blocks, stairways)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_blocks_stairways.eng";

   const std::string_view expected_eng =
      R"(BlocksStairways(3)
{
   Stairway()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      StepHeight(0.1);
      FirstStepOffset(0);
      SurfaceMaterials(0, 1, 2, 3, 4, 5);
      SurfaceTextureMode(0, 0, 0, 0, 0, 0);
      SurfaceTextureRotation(2, 2, 2, 2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 256, 256, 0, 0);
   }
   Stairway()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      StepHeight(0.25);
      FirstStepOffset(0.125);
      SurfaceMaterials(2, 2, 2, 2, 2, 2);
      SurfaceTextureMode(1, 1, 1, 1, 5, 1);
      SurfaceTextureRotation(1, 1, 1, 1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
   Stairway()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      StepHeight(1);
      FirstStepOffset(0);
      SurfaceMaterials(0, 0, 0, 0, 0, 0);
      SurfaceTextureMode(2, 2, 2, 2, 2, 2);
      SurfaceTextureRotation(0, 0, 0, 0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
}

)";

   world::entity_group
      group =
         {
            .blocks =
               {
                  .stairways =
                     {
                        .description =
                           {
                              world::block_description_stairway{
                                 .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                                 .position = {8.5f, 4.5f, 2.0f},
                                 .size = {4.0f, 4.0f, 4.0f},
                                 .step_height = 0.1f,
                                 .first_step_offset = 0.0f,
                                 .surface_materials = {0, 1, 2, 3, 4, 5},
                                 .surface_texture_mode =
                                    {
                                       block_texture_mode::world_space_auto,
                                       block_texture_mode::world_space_auto,
                                       block_texture_mode::world_space_auto,
                                       block_texture_mode::world_space_auto,
                                       block_texture_mode::world_space_auto,
                                       block_texture_mode::world_space_auto,
                                    },
                                 .surface_texture_rotation =
                                    {
                                       block_texture_rotation::d180,
                                       block_texture_rotation::d180,
                                       block_texture_rotation::d180,
                                       block_texture_rotation::d180,
                                       block_texture_rotation::d180,
                                       block_texture_rotation::d180,
                                    },
                                 .surface_texture_scale =
                                    {
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{-1, -2},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                    },
                                 .surface_texture_offset =
                                    {
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{256, 256},
                                       std::array<uint16, 2>{0, 0},
                                    },
                              },
                              world::block_description_stairway{
                                 .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                                 .position = {10.0f, 16.0f, 12.0f},
                                 .size = {8.0f, 4.0f, 8.0f},
                                 .step_height = 0.25f,
                                 .first_step_offset = 0.125f,
                                 .surface_materials = {2, 2, 2, 2, 2, 2},
                                 .surface_texture_mode =
                                    {
                                       block_texture_mode::world_space_zy,
                                       block_texture_mode::world_space_zy,
                                       block_texture_mode::world_space_zy,
                                       block_texture_mode::world_space_zy,
                                       block_texture_mode::local_space_zy,
                                       block_texture_mode::world_space_zy,
                                    },
                                 .surface_texture_rotation =
                                    {
                                       block_texture_rotation::d90,
                                       block_texture_rotation::d90,
                                       block_texture_rotation::d90,
                                       block_texture_rotation::d90,
                                       block_texture_rotation::d90,
                                       block_texture_rotation::d90,
                                    },
                                 .surface_texture_scale =
                                    {
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{-2, -2},
                                    },
                                 .surface_texture_offset =
                                    {
                                       std::array<uint16, 2>{1024, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                    },
                              },
                              world::block_description_stairway{
                                 .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                                 .position = {6.0f, 6.0f, 6.0f},
                                 .size = {5.0f, 5.0f, 5.0f},
                                 .step_height = 1.0f,
                                 .first_step_offset = 0.0f,
                                 .surface_materials = {0, 0, 0, 0, 0, 0},
                                 .surface_texture_mode =
                                    {
                                       block_texture_mode::world_space_xz,
                                       block_texture_mode::world_space_xz,
                                       block_texture_mode::world_space_xz,
                                       block_texture_mode::world_space_xz,
                                       block_texture_mode::world_space_xz,
                                       block_texture_mode::world_space_xz,
                                    },
                                 .surface_texture_rotation =
                                    {
                                       block_texture_rotation::d0,
                                       block_texture_rotation::d0,
                                       block_texture_rotation::d0,
                                       block_texture_rotation::d0,
                                       block_texture_rotation::d0,
                                       block_texture_rotation::d0,
                                    },
                                 .surface_texture_scale =
                                    {
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                       std::array<int8, 2>{0, 0},
                                    },
                                 .surface_texture_offset =
                                    {
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                       std::array<uint16, 2>{0, 0},
                                    },
                              },
                           },

                        .mesh =
                           {
                              blocks_custom_mesh_library::null_handle(),
                              blocks_custom_mesh_library::null_handle(),
                              blocks_custom_mesh_library::null_handle(),
                           },
                     },

               },

         };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

TEST_CASE("world save entity group (blocks, materials)", "[World][IO]")
{
   (void)io::create_directory("temp/entity_groups");
   const io::path path = "temp/entity_groups/test_blocks_materials.eng";

   const std::string_view expected_eng =
      R"(BlocksMaterials(2)
{
   Material()
   {
      Name("rocks");
      DiffuseMap("rocks_diffuse");
      NormalMap("rocks_normal");
      DetailMap("rocks_detail");
      EnvMap("skycube");
      DetailTiling(4, 4);
      TileNormalMap(0);
      SpecularLighting(1);
      SpecularColor(0.5, 0.5, 0.5);
      FoleyFXGroup(0);
   }
   Material()
   {
      Name("snow");
      DiffuseMap("snow_diffuse");
      NormalMap("snow_detail_normal");
      DetailMap("snow_detail");
      EnvMap("skycube");
      DetailTiling(3, 3);
      TileNormalMap(1);
      SpecularLighting(1);
      SpecularColor(0.75, 0.75, 0.75);
      FoleyFXGroup(4);
   }
}

)";

   world::entity_group group = {
      .blocks =
         {
            .materials =
               {
                  world::block_material{
                     .name = "rocks",

                     .diffuse_map = "rocks_diffuse",
                     .normal_map = "rocks_normal",
                     .detail_map = "rocks_detail",
                     .env_map = "skycube",

                     .detail_tiling = {4, 4},
                     .tile_normal_map = false,
                     .specular_lighting = true,

                     .specular_color = {0.5f, 0.5f, 0.5f},
                  },

                  world::block_material{
                     .name = "snow",

                     .diffuse_map = "snow_diffuse",
                     .normal_map = "snow_detail_normal",
                     .detail_map = "snow_detail",
                     .env_map = "skycube",

                     .detail_tiling = {3, 3},
                     .tile_normal_map = true,
                     .specular_lighting = true,

                     .specular_color = {0.75f, 0.75f, 0.75f},

                     .foley_group = block_foley_group::snow,
                  },

               },
         },
   };

   world::save_entity_group(path, group);

   const auto written_eng = io::read_file_to_string(path);

   CHECK(written_eng == expected_eng);
}

}