
#include "pch.h"

#include "utility/read_file.hpp"
#include "world/world_io_save.hpp"

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

GlobalLights()
{
	EditorGlobalDirIconSize(10);
	Light1("sun");
	Light2("");
	Top(255, 255, 255);
	Bottom(0, 0, 0);
}
)"sv;

constexpr auto expected_pth = R"(Version(10);
PathCount(1);

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

)"sv;

constexpr auto expected_rgn = R"(Version(1);
RegionCount(1);

Region("lightregion2", 2)
{
	Position(-188.456131, -2.336851, -7.291678);
	Rotation(0.546265, 0.038489, 0.027578, 0.836273);
	Size(0.100000, 10.000000, 3.284148);
	Name("lightregion2");
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
	Corner(72.596146, 0.000000, -31.159695);
	Corner(86.691795, 0.000000, -0.198154);
	Corner(99.806587, 0.000000, -6.168838);
	Corner(85.710938, 0.000000, -37.130379);
	Flag(32);
}

)"sv;

constexpr auto expected_bnd = R"(Boundary()
{
	Path("boundary");
}

)"sv;

}

TEST_CASE("world saving", "[World][IO]")
{
   std::filesystem::create_directory(L"temp/world");

   const world world{
      .lighting_settings = {.global_lights = {"sun", ""},
                            .ambient_sky_color = {1.0f, 1.0f, 1.0f},
                            .ambient_ground_color = {0.0f, 0.0f, 0.0f}},

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
      }},

      .paths = {path{
         .name = "The Amazing Path",
         .spline_type = path_spline_type::catmull_rom,
         .properties = {path::property{.key = "FloatVal", .value = "5.0000"},
                        path::property{.key = "StringVal", .value = "Such string, much wow"}},

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

      }},

      .regions = {region{.name = "lightregion2",
                         .rotation = {0.027578f, -0.836273f, 0.546265f, -0.038489f},
                         .position = {-188.456131f, -2.336851f, 7.291678f},
                         .size = {0.100000f, 10.0f, 3.284148f},
                         .shape = region_shape::cylinder,
                         .description = "lightregion2"}},

      .sectors = {sector{.name = "sector-1",
                         .base = 0.0f,
                         .height = 10.0f,
                         .points = {{-157.581009f, -4.900336},
                                    {-227.199097f, -7.364827},
                                    {-228.642029f, 40.347687},
                                    {-159.451279f, 40.488800}},
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

      // TODO: Path planning.

      .barriers = {barrier{.name = "Barrier0",
                           .corners =
                              {
                                 float2{72.596146f, 31.159695f},
                                 float2{86.691795f, 0.198154f},
                                 float2{99.806587f, 6.168838f},
                                 float2{85.710938f, 37.130379f},
                              },
                           .flags = ai_path_flags::flyer}},

      .boundaries = {{.name = "boundary"}}};

   save_world(L"temp/world/test.wld", world);

   const auto written_wld =
      utility::read_file_to_string(L"temp/world/test.wld");

   CHECK(written_wld == expected_wld);

   const auto written_lgt =
      utility::read_file_to_string(L"temp/world/test.lgt");

   CHECK(written_lgt == expected_lgt);

   const auto written_pth =
      utility::read_file_to_string(L"temp/world/test.pth");

   CHECK(written_pth == expected_pth);

   const auto written_rgn =
      utility::read_file_to_string(L"temp/world/test.rgn");

   CHECK(written_rgn == expected_rgn);

   const auto written_pvs =
      utility::read_file_to_string(L"temp/world/test.pvs");

   CHECK(written_pvs == expected_pvs);

   const auto written_hnt =
      utility::read_file_to_string(L"temp/world/test.hnt");

   CHECK(written_hnt == expected_hnt);

   const auto written_bar =
      utility::read_file_to_string(L"temp/world/test.bar");

   CHECK(written_bar == expected_bar);

   const auto written_bnd =
      utility::read_file_to_string(L"temp/world/test.bnd");

   CHECK(written_bnd == expected_bnd);
}
}
