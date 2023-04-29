
#include "pch.h"

#include "io/output_file.hpp"
#include "io/read_file.hpp"
#include "world/world_io_save.hpp"

#include <fmt/core.h>

namespace we::world::tests {

using namespace std::literals;

namespace {

constexpr auto expected_wld = R"(Version(3);
SaveType(0);

LightName("test.LGT");
TerrainName("test.ter");
SkyName("test.sky");

NextSequence(1);

Object("object", "bldg_real_object", 0)
{
	ChildRotation(0.691417, 0.203867, 0.659295, -0.213800);
	ChildPosition(-136.000000, 0.000000, -24.000000);
	Team(0);
	NetworkId(-1);
	SomeProperty("An Amazing Value");
}

)"sv;

constexpr auto expected_lgt = R"(Light("sun", 0)
{
	Rotation(0.922542, 0.383802, -0.039506, -0.008607);
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

Light("Region Light", 1)
{
	Rotation(0.581487, 0.314004, 0.435918, -0.610941);
	Position(-216.604019, 2.231649, -18.720726);
	Type(1);
	Color(1.000000, 0.501961, 0.501961);
	Static();
	Region("lightregion");
	PS2BlendMode(0);
	TileUV(1.000000, 1.000000);
	OffsetUV(0.000000, 0.000000);
}

GlobalLights()
{
	EditorGlobalDirIconSize(10);
	Light1("sun");
	Light2("");
	Top(255, 255, 255);
	Bottom(128, 128, 128);
}
)"sv;

constexpr auto expected_pth = R"(Version(10);
PathCount(3);

Path("The Amazing Path")
{
	Data(1);
	PathType(0);
	PathSpeedType(0);
	PathTime(0.000000);
	OffsetPath(0);
	SplineType("Catmull-Rom");

	Properties(2)
	{
		FloatVal(5.0000);
		StringVal("Such string, much wow");
	}

	Nodes(2)
	{
		Node()
		{
			Position(-2.006914, -0.002500, 56.238541);
			Knot(0.000000);
			Data(1);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(2)
			{
				FloatVal(5.0000);
				StringVal("Such string, much wow");
			}
		}

		Node()
		{
			Position(-0.484701, -0.002500, 54.641960);
			Knot(0.000000);
			Data(1);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(0.703845, 0.000000, 0.710354, 0.000000);
			Properties(0)
			{
			}
		}

	}

}

Path("type_EntityPath The Other Amazing Path")
{
	Data(1);
	PathType(0);
	PathSpeedType(0);
	PathTime(0.000000);
	OffsetPath(0);
	SplineType("Catmull-Rom");

	Properties(2)
	{
		FloatVal(5.0000);
		StringVal("Such string, much wow");
	}

	Nodes(2)
	{
		Node()
		{
			Position(-2.006914, -0.002500, 56.238541);
			Knot(0.000000);
			Data(1);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(2)
			{
				FloatVal(5.0000);
				StringVal("Such string, much wow");
			}
		}

		Node()
		{
			Position(-0.484701, -0.002500, 54.641960);
			Knot(0.000000);
			Data(1);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(0.703845, 0.000000, 0.710354, 0.000000);
			Properties(0)
			{
			}
		}

	}

}

Path("boundary")
{
	Data(0);
	PathType(0);
	PathSpeedType(0);
	PathTime(0.000000);
	OffsetPath(0);
	SplineType("Hermite");

	Properties(0)
	{
	}

	Nodes(12)
	{
		Node()
		{
			Position(383.557434, 0.000000, -4.797791);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(332.111023, 0.000000, 187.202209);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(191.557434, 0.000000, 327.755798);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(-0.442566, 0.000000, 379.202209);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(-192.442566, 0.000000, 327.755798);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(-332.996155, 0.000000, 187.202209);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(-384.442566, 0.000000, -4.797791);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(-332.996155, 0.000000, -196.797791);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(-192.442566, 0.000000, -337.351379);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(-0.442566, 0.000000, -388.797791);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(191.557434, 0.000000, -337.351379);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

		Node()
		{
			Position(332.111023, 0.000000, -196.797791);
			Knot(0.000000);
			Data(0);
			Time(1.000000);
			PauseTime(0.000000);
			Rotation(1.000000, 0.000000, 0.000000, 0.000000);
			Properties(0)
			{
			}
		}

	}

}

)"sv;

constexpr auto expected_rgn = R"(Version(1);
RegionCount(2);

Region("lightregion2", 2)
{
	Position(-188.456131, -2.336851, -7.291678);
	Rotation(0.546265, 0.038489, 0.027578, 0.836273);
	Size(0.100000, 10.000000, 3.284148);
	Name("lightregion2");
}

Region("lightregion", 1)
{
	Position(-216.604019, 2.231649, -18.720726);
	Rotation(1.000000, -0.000000, 0.000000, -0.000000);
	Size(4.591324, 0.100000, 1.277475);
	Name("lightregion");
}

)"sv;

constexpr auto expected_pvs = R"(Sector("sector-1")
{
	Base(0.000000);
	Height(10.000000);
	Point(-157.581009, 4.900336);
	Point(-227.199097, 7.364827);
	Point(-228.642029, -40.347687);
	Point(-159.451279, -40.488800);
	Object("really_complex_object");
}
Sector("sector-2")
{
	Base(0.000000);
	Height(10.000000);
	Point(-196.648041, -125.908623);
	Point(-195.826218, -49.666763);
	Point(-271.034851, -48.864563);
	Point(-274.260132, -128.690567);
	Object("also_complex_object");
}
Portal("Portal")
{
	Rotation(0.546265, 0.038489, 0.027578, 0.836273);
	Position(-193.661575, 2.097009, -31.728502);
	Width(2.920000);
	Height(4.120000);
	Sector1("sector-1");
	Sector2("sector-2");
}
)"sv;

constexpr auto expected_hnt = R"(Hint("HintNode0", "5")
{
	Position(-70.045296, 1.000582, -19.298828);
	Rotation(0.303753, 0.399004, -0.569245, -0.651529);
	Radius(7.692307);
	Mode(0);
	CommandPost("cp2");
}
)"sv;

constexpr auto expected_bar = R"(BarrierCount(1);

Barrier("Barrier0")
{
	Corner(72.596146, 1.000000, -31.159695);
	Corner(86.691795, 1.000000, -0.198154);
	Corner(99.806595, 1.000000, -6.168838);
	Corner(85.710945, 1.000000, -37.130379);
	Flag(32);
}

)"sv;

constexpr auto expected_pln = R"(
Hub("Hub0")
{
	Pos(-63.822487, 0.000000, 9.202278);
	Radius(8.000000);
}

Hub("Hub1")
{
	Pos(-121.883095, 1.000000, 30.046543);
	Radius(7.586431);
	BranchWeight("Hub0",100.000000,"Connection0",32);
	BranchWeight("Hub0",75.000000,"Connection0",16);
	BranchWeight("Hub0",25.000000,"Connection0",8);
	BranchWeight("Hub0",7.500000,"Connection0",4);
	BranchWeight("Hub0",15.000000,"Connection0",2);
	BranchWeight("Hub0",20.000000,"Connection0",1);
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
)"sv;

constexpr auto expected_bnd = R"(Boundary()
{
	Path("boundary");
}

)"sv;

constexpr auto expected_ldx = R"(Version(1);
NextID(2);

Layer("[Base]", 0, 0)
{
	Description("");
}

Layer("conquest", 1, 0)
{
	Description("");
}

GameMode("Common")
{
	Layer(0);
}

GameMode("conquest")
{
	Layer(1);
}

)"sv;

constexpr auto expected_req = R"(ucft
{
	REQN
	{
		"path"
		"test"
	}
	REQN
	{
		"congraph"
		"test"
	}
	REQN
	{
		"envfx"
		"test"
	}
	REQN
	{
		"world"
		"test"
	}
	REQN
	{
		"prop"
		"test"
	}
	REQN
	{
		"povs"
		"test"
	}
	REQN
	{
		"lvl"
		"test_conquest"
	}
}
)"sv;

constexpr auto expected_mrq = R"(ucft
{
	REQN
	{
		"world"
		"test_conquest"
	}
}
)"sv;

}

TEST_CASE("world saving", "[World][IO]")
{
   std::filesystem::create_directory(L"temp/world");

   id_generator<planning_hub> hub_id_generator;

   const std::array<planning_hub_id, 4> hub_ids = {hub_id_generator.aquire(),
                                                   hub_id_generator.aquire(),
                                                   hub_id_generator.aquire(),
                                                   hub_id_generator.aquire()};

   const world world{
      .name = "test",

      .requirements =
         {
            {.file_type = "path", .entries = {"test"}},
            {.file_type = "congraph", .entries = {"test"}},
            {.file_type = "envfx", .entries = {"test"}},
            {.file_type = "world", .entries = {"test"}},
            {.file_type = "prop", .entries = {"test"}},
            {.file_type = "povs", .entries = {"test"}},
            {.file_type = "lvl", .entries = {"test_conquest"}},
         },

      .layer_descriptions = {{.name = "[Base]"}, {.name = "conquest"}},

      .game_modes = {{.name = "Common", .layers = {0}},

                     {.name = "conquest",
                      .layers = {1},
                      .requirements =
                         {
                            {.file_type = "world", .entries = {"test_conquest"}},
                         }}},

      .global_lights = {.global_light_1 = "sun",
                        .global_light_2 = "",
                        .ambient_sky_color = {1.0f, 1.0f, 1.0f},
                        .ambient_ground_color = {0.5f, 0.5f, 0.5f}},

      .objects = {object{.name = "object",

                         .rotation = {0.659295f, 0.2138f, 0.691417f, -0.203867f},
                         .position = {-136.0f, 0.0f, 24.0f},

                         .team = 0,

                         .class_name = lowercase_string{"bldg_real_object"sv},
                         .instance_properties = {{.key = "SomeProperty", .value = "An Amazing Value"}}}},

      .lights = {light{
                    .name = "sun",

                    .rotation = {-0.039506f, 0.008607f, 0.922542f, -0.383802f},
                    .position = {-159.264923f, 19.331013f, 66.727310f},
                    .color = {1.0f, 0.882353f, 0.752941f},
                    .static_ = true,
                    .shadow_caster = true,
                    .specular_caster = true,
                    .light_type = light_type::directional,
                 },

                 light{
                    .name = "Region Light",

                    .rotation = {0.435918f, 0.610941f, 0.581487f, -0.314004f},
                    .position = {-216.604019f, 2.231649f, 18.720726f},
                    .color = {1.0f, 0.501961f, 0.501961f},
                    .static_ = true,
                    .light_type = light_type::directional_region_sphere,
                    .region_name = "lightregion",
                    .region_size = {4.591324f, 0.100000f, 1.277475f},
                    .region_rotation = {0.0f, 0.0f, 1.0f, 0.0f},
                 }},

      .paths =
         {path{.name = "The Amazing Path",
               .spline_type = path_spline_type::catmull_rom,
               .properties = {path::property{.key = "FloatVal", .value = "5.0000"},
                              path::property{.key = "StringVal",
                                             .value = "Such string, much wow"}},

               .nodes = {path::node{
                            .rotation = {0.0f, -0.0f, 1.0f, -0.0f},
                            .position = {-2.006914f, -0.002500f, -56.238541f},
                            .properties = {{.key = "FloatVal", .value = "5.0000"},
                                           {.key = "StringVal", .value = "Such string, much wow"}},
                         },

                         path::node{
                            .rotation = {0.710354f, -0.0f, 0.703845f, -0.0f},
                            .position = {-0.484701f, -0.002500f, -54.641960f},
                         }}

          },

          path{.name = "The Other Amazing Path",
               .type = path_type::entity_follow,
               .spline_type = path_spline_type::catmull_rom,
               .properties = {path::property{.key = "FloatVal", .value = "5.0000"},
                              path::property{.key = "StringVal",
                                             .value = "Such string, much wow"}},

               .nodes = {path::node{
                            .rotation = {0.0f, -0.0f, 1.0f, -0.0f},
                            .position = {-2.006914f, -0.002500f, -56.238541f},
                            .properties = {{.key = "FloatVal", .value = "5.0000"},
                                           {.key = "StringVal", .value = "Such string, much wow"}},
                         },

                         path::node{
                            .rotation = {0.710354f, -0.0f, 0.703845f, -0.0f},
                            .position = {-0.484701f, -0.002500f, -54.641960f},
                         }}}},

      .regions = {region{.name = "lightregion2",
                         .rotation = {0.027578f, -0.836273f, 0.546265f, -0.038489f},
                         .position = {-188.456131f, -2.336851f, 7.291678f},
                         .size = {0.100000f, 10.0f, 3.284148f},
                         .shape = region_shape::cylinder,
                         .description = "lightregion2"}},

      .sectors = {sector{.name = "sector-1",
                         .base = 0.0f,
                         .height = 10.0f,
                         .points = {{-157.581009f, -4.900336f},
                                    {-227.199097f, -7.364827f},
                                    {-228.642029f, 40.347687f},
                                    {-159.451279f, 40.488800f}},
                         .objects = {"really_complex_object"}},

                  sector{.name = "sector-2",
                         .base = 0.0f,
                         .height = 10.0f,
                         .points = {{-196.648041f, 125.908623f},
                                    {-195.826218f, 49.666763f},
                                    {-271.034851f, 48.864563f},
                                    {-274.260132f, 128.690567f}},
                         .objects = {"also_complex_object"}}},

      .portals = {portal{.name = "Portal",
                         .rotation = {0.027578f, -0.836273f, 0.546265f, -0.038489f},
                         .position = {-193.661575f, 2.097009f, 31.728502f},
                         .width = 2.92f,
                         .height = 4.12f,
                         .sector1 = "sector-1",
                         .sector2 = "sector-2"}},

      .hintnodes = {hintnode{.name = "HintNode0",
                             .rotation = {-0.569245f, 0.651529f, 0.303753f, -0.399004f},
                             .position = {-70.045296f, 1.000582f, 19.298828f},
                             .type = hintnode_type::mine,
                             .mode = hintnode_mode::none,
                             .radius = 7.692307f,
                             .command_post = "cp2"}},

      .barriers = {barrier{.name = "Barrier0",
                           .position = {86.2013702f, 1.0f, 18.6642666f},
                           .size = {7.20497799f, 17.0095882f},
                           .rotation_angle = {2.71437049f},
                           .flags = ai_path_flags::flyer}},

      .planning_hubs = {planning_hub{.name = "Hub0",
                                     .position = float3{-63.822487f, 0.0f, -9.202278f},
                                     .radius = 8.0f,
                                     .id = hub_ids[0]},

                        planning_hub{.name = "Hub1",
                                     .position = float3{-121.883095f, 1.0f, -30.046543f},
                                     .radius = 7.586431f,
                                     .id = hub_ids[1]},

                        planning_hub{.name = "Hub2",
                                     .position = float3{-54.011314f, 2.0f, -194.037018f},
                                     .radius = 13.120973f,
                                     .id = hub_ids[2]},

                        planning_hub{.name = "Hub3",
                                     .position = float3{-163.852570f, 3.0f, -169.116760f},
                                     .radius = 12.046540f,
                                     .id = hub_ids[3]}},

      .planning_connections =
         {planning_connection{.name = "Connection0",
                              .start = hub_ids[0],
                              .end = hub_ids[1],
                              .flags = (ai_path_flags::soldier | ai_path_flags::hover |
                                        ai_path_flags::small | ai_path_flags::medium |
                                        ai_path_flags::huge | ai_path_flags::flyer),

                              .backward_weights = {.soldier = 20.0f,
                                                   .hover = 15.0f,
                                                   .small = 7.5f,
                                                   .medium = 25.0f,
                                                   .huge = 75.0f,
                                                   .flyer = 100.0f}},

          planning_connection{.name = "Connection1",
                              .start = hub_ids[3],
                              .end = hub_ids[2],
                              .flags = ai_path_flags::hover}},

      .boundaries = {{.name = "boundary",
                      .position = {-0.442565918f, 4.79779053f},
                      .size = {384.000000f, 384.000000f}}},

      .planning_hub_index = {{hub_ids[0], 0},
                             {hub_ids[1], 1},
                             {hub_ids[2], 2},
                             {hub_ids[3], 3}}};

   save_world(L"temp/world/test.wld", world);

   const auto written_wld = io::read_file_to_string(L"temp/world/test.wld");

   CHECK(written_wld == expected_wld);

   const auto written_lgt = io::read_file_to_string(L"temp/world/test.lgt");

   CHECK(written_lgt == expected_lgt);

   const auto written_pth = io::read_file_to_string(L"temp/world/test.pth");

   CHECK(written_pth == expected_pth);

   const auto written_rgn = io::read_file_to_string(L"temp/world/test.rgn");

   CHECK(written_rgn == expected_rgn);

   const auto written_pvs = io::read_file_to_string(L"temp/world/test.pvs");

   CHECK(written_pvs == expected_pvs);

   const auto written_hnt = io::read_file_to_string(L"temp/world/test.hnt");

   CHECK(written_hnt == expected_hnt);

   const auto written_bar = io::read_file_to_string(L"temp/world/test.bar");

   CHECK(written_bar == expected_bar);

   const auto written_pln = io::read_file_to_string(L"temp/world/test.pln");

   CHECK(written_pln == expected_pln);

   const auto written_bnd = io::read_file_to_string(L"temp/world/test.bnd");

   CHECK(written_bnd == expected_bnd);

   const auto written_ldx = io::read_file_to_string(L"temp/world/test.ldx");

   CHECK(written_ldx == expected_ldx);

   const auto written_req = io::read_file_to_string(L"temp/world/test.req");

   CHECK(written_req == expected_req);

   const auto written_mrq =
      io::read_file_to_string(L"temp/world/test_conquest.mrq");

   CHECK(written_mrq == expected_mrq);
}

TEST_CASE("world saving garbage collect", "[World][IO]")
{
   std::filesystem::create_directory(L"temp/world_gc");

   const std::array<std::string_view, 5> layer_files{"lyr", "pth", "rgn", "lgt", "hnt"};

   const world world{
      .name = "test",

      .deleted_layers = {"conquest", "ctf", "sound"},
      .deleted_game_modes = {"conquest", "ctf"},
   };

   for (const auto& layer : world.deleted_layers) {
      for (const auto& file : layer_files) {
         [[maybe_unused]] io::output_file output_file{
            fmt::format("temp/world_gc/test_{}.{}", layer, file)};
      }
   }

   // Game Modes
   {
      [[maybe_unused]] io::output_file file{"temp/world_gc/test_conquest.mrq"};

      // NB: Test that test_ctf.mrq already being gone causes no issues.
   }

   save_world(L"temp/world_gc/test.wld", world);

   for (const auto& layer : world.deleted_layers) {
      for (const auto& file : layer_files) {
         CHECK(not std::filesystem::exists(
            fmt::format("temp/world_gc/test_{}.{}", layer, file)));
      }
   }

   for (const auto& game_mode : world.deleted_game_modes) {
      CHECK(not std::filesystem::exists(
         fmt::format("temp/world_gc/test_{}.mrq", game_mode)));
   }
}

}
