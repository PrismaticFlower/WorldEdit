
#include "pch.h"

#include "edits/add_block.hpp"

#include "io/output_file.hpp"
#include "io/read_file.hpp"

#include "world/io/save.hpp"

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
	Texture("sun_shadow", 1);
	CastSpecular(1);
	PS2BlendMode(2);
	TileUV(1.000000, 1.000000);
	OffsetUV(0.000000, 0.000000);
}

Light("Light 3", 1)
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

Light("Region Light", 2)
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
PathCount(4);

Path("The Amazing Path")
{
	Data(1);
	PathType(0);
	PathSpeedType(0);
	PathTime(0.000000);
	OffsetPath(0);
	SplineType("Catmull-Rom");

	Properties(3)
	{
		FloatVal(5.0000);
		StringVal("Such string, much wow");
		EmptyVal("");
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
			Properties(3)
			{
				FloatVal(5.0000);
				StringVal("Such string, much wow");
				EmptyVal("");
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

Path("boundary1")
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
			Position(383.557434, 1.000000, -4.797791);
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
			Position(332.111023, 1.000000, 187.202209);
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
			Position(191.557434, 1.000000, 327.755798);
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
			Position(-0.442566, 1.000000, 379.202209);
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
			Position(-192.442566, 1.000000, 327.755798);
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
			Position(-332.996155, 1.000000, 187.202209);
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
			Position(-384.442566, 1.000000, -4.797791);
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
			Position(-332.996155, 1.000000, -196.797791);
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
			Position(-192.442566, 1.000000, -337.351379);
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
			Position(-0.442566, 1.000000, -388.797791);
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
			Position(191.557434, 1.000000, -337.351379);
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
			Position(332.111023, 1.000000, -196.797791);
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

constexpr auto expected_hnt = R"(
Hint("HintNode0", "5")
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
	Dynamic(1);
	Jump();
	JetJump();
	OneWay();
}
)"sv;

constexpr auto expected_bnd = R"(Boundary()
{
	Path("boundary");
	Path("boundary1");
}

)"sv;

constexpr auto expected_msr = R"(MeasurementCount(2);

Measurement("Measurement0")
{
	Start(1.000000, 0.000000, 0.000000);
	End(2.000000, 0.000000, 1.000000);
}

Measurement("")
{
	Start(1.000000, 0.000000, -5.500000);
	End(-1.000000, 0.000000, 0.000000);
}

)"sv;

constexpr auto expected_anm = R"(Animation("Anim", 10.00, 0, 1)
{
	AddPositionKey(0.00, 10.00, 0.00, 0.00, 0, -0.00, 0.00, 0.00, -0.00, 0.00, 0.00);
	AddPositionKey(5.00, 50.00, 30.00, 78.00, 1, -0.00, 0.00, 0.00, -0.00, 0.00, 0.00);
	AddPositionKey(10.00, 60.00, 30.00, 78.00, 2, -10.00, 0.00, 0.00, -0.00, 0.00, 10.00);
	AddRotationKey(0.00, 0.00, -0.00, -0.00, 1, 0.00, -0.00, -0.00, 0.00, -0.00, -0.00);
	AddRotationKey(5.00, 0.00, -45.00, -0.00, 2, 35.00, -0.00, -0.00, 0.00, -0.00, -35.00);
	AddRotationKey(7.50, 0.00, -90.00, -0.00, 0, 0.00, -0.00, -0.00, 0.00, -0.00, -0.00);
}

AnimationGroup("group", 1, 0)
{
	DisableHierarchies();
	Animation("Anim", "com_inv_col_8");
}

Hierarchy("com_inv_col_8")
{
	Obj("com_item_healthrecharge");
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

constexpr auto expected_ldx_no_gamemodes = R"(Version(1);
NextID(2);

Layer("[Base]", 0, 0)
{
	Description("");
}

Layer("conquest", 1, 0)
{
	Description("");
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

constexpr auto expected_pth_no_boundary_spline_type = R"(Version(10);
PathCount(2);

Path("boundary")
{
	Data(0);
	PathType(2);
	PathSpeedType(0);
	PathTime(0.000000);
	OffsetPath(0);

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

Path("boundary1")
{
	Data(0);
	PathType(2);
	PathSpeedType(0);
	PathTime(0.000000);
	OffsetPath(0);

	Properties(0)
	{
	}

	Nodes(12)
	{
		Node()
		{
			Position(383.557434, 1.000000, -4.797791);
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
			Position(332.111023, 1.000000, 187.202209);
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
			Position(191.557434, 1.000000, 327.755798);
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
			Position(-0.442566, 1.000000, 379.202209);
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
			Position(-192.442566, 1.000000, 327.755798);
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
			Position(-332.996155, 1.000000, 187.202209);
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
			Position(-384.442566, 1.000000, -4.797791);
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
			Position(-332.996155, 1.000000, -196.797791);
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
			Position(-192.442566, 1.000000, -337.351379);
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
			Position(-0.442566, 1.000000, -388.797791);
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
			Position(191.557434, 1.000000, -337.351379);
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
			Position(332.111023, 1.000000, -196.797791);
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
}

TEST_CASE("world saving", "[World][IO]")
{
   (void)io::create_directory("temp/world");

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

      .game_modes = {{.name = "conquest",
                      .layers = {1},
                      .requirements =
                         {
                            {.file_type = "world", .entries = {"test_conquest"}},
                         }}},
      .common_layers = {0},

      .global_lights = {.global_light_1 = "sun",
                        .global_light_2 = "",
                        .ambient_sky_color = {1.0f, 1.0f, 1.0f},
                        .ambient_ground_color = {0.5f, 0.5f, 0.5f}},

      .objects = {entities_init,
                  std::initializer_list{
                     object{.name = "object",

                            .rotation = {0.659295f, 0.2138f, 0.691417f, -0.203867f},
                            .position = {-136.0f, 0.0f, 24.0f},

                            .team = 0,

                            .class_name = lowercase_string{"bldg_real_object"sv},
                            .instance_properties = {{.key = "SomeProperty", .value = "An Amazing Value"}}},
                  }},

      .lights = {entities_init,
                 std::initializer_list{
                    light{
                       .name = "sun",

                       .rotation = {-0.039506f, 0.008607f, 0.922542f, -0.383802f},
                       .position = {-159.264923f, 19.331013f, 66.727310f},
                       .color = {1.0f, 0.882353f, 0.752941f},
                       .static_ = true,
                       .shadow_caster = true,
                       .specular_caster = true,
                       .light_type = light_type::directional,
                       .texture_addressing = texture_addressing::clamp,
                       .ps2_blend_mode = ps2_blend_mode::blend,
                       .texture = "sun_shadow",
                    },

                    light{
                       .name = "Light 3",

                       .rotation = {0.0f, -0.0f, 1.0f, -0.0f},
                       .position = {-149.102463f, 0.469788f, -22.194153f},
                       .color = {1.0f, 1.0f, 1.0f},
                       .static_ = true,
                       .shadow_caster = true,
                       .specular_caster = false,
                       .light_type = light_type::spot,
                       .bidirectional = true,
                       .range = 5.0f,
                       .inner_cone_angle = 0.785398f,
                       .outer_cone_angle = 0.872665f,
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
                    },
                 }},

      .paths =
         {entities_init,
          std::initializer_list{
             path{.name = "The Amazing Path",
                  .spline_type = path_spline_type::catmull_rom,
                  .properties = {{.key = "FloatVal", .value = "5.0000"},
                                 {.key = "StringVal", .value = "Such string, much wow"},
                                 {.key = "EmptyVal", .value = ""}},

                  .nodes = {path::node{
                               .rotation = {0.0f, -0.0f, 1.0f, -0.0f},
                               .position = {-2.006914f, -0.002500f, -56.238541f},
                               .properties = {{.key = "FloatVal", .value = "5.0000"},
                                              {.key = "StringVal", .value = "Such string, much wow"},
                                              {.key = "EmptyVal", .value = ""}},
                            },

                            path::node{
                               .rotation = {0.710354f, -0.0f, 0.703845f, -0.0f},
                               .position = {-0.484701f, -0.002500f, -54.641960f},
                            }}

             },

             path{.name = "The Other Amazing Path",
                  .type = path_type::entity_follow,
                  .spline_type = path_spline_type::catmull_rom,
                  .properties = {{.key = "FloatVal", .value = "5.0000"},
                                 {.key = "StringVal", .value = "Such string, much wow"}},

                  .nodes = {path::node{
                               .rotation = {0.0f, -0.0f, 1.0f, -0.0f},
                               .position = {-2.006914f, -0.002500f, -56.238541f},
                               .properties = {{.key = "FloatVal", .value = "5.0000"},
                                              {.key = "StringVal", .value = "Such string, much wow"}},
                            },

                            path::node{
                               .rotation = {0.710354f, -0.0f, 0.703845f, -0.0f},
                               .position = {-0.484701f, -0.002500f, -54.641960f},
                            }}},
          }},

      .regions = {entities_init,
                  std::initializer_list{
                     region{.name = "lightregion2",
                            .rotation = {0.027578f, -0.836273f, 0.546265f, -0.038489f},
                            .position = {-188.456131f, -2.336851f, 7.291678f},
                            .size = {0.100000f, 10.0f, 3.284148f},
                            .shape = region_shape::cylinder,
                            .description = "lightregion2"},
                  }},

      .sectors = {entities_init,
                  std::initializer_list{
                     sector{.name = "sector-1",
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
                            .objects = {"also_complex_object"}},
                  }},

      .portals =
         {
            entities_init,
            std::initializer_list{
               portal{.name = "Portal",
                      .rotation = {0.027578f, -0.836273f, 0.546265f, -0.038489f},
                      .position = {-193.661575f, 2.097009f, 31.728502f},
                      .width = 2.92f,
                      .height = 4.12f,
                      .sector1 = "sector-1",
                      .sector2 = "sector-2"},
            },
         },

      .hintnodes =
         {
            entities_init,
            std::initializer_list{
               hintnode{.name = "HintNode0",
                        .rotation = {-0.569245f, 0.651529f, 0.303753f, -0.399004f},
                        .position = {-70.045296f, 1.000582f, 19.298828f},
                        .type = hintnode_type::mine,
                        .mode = hintnode_mode::none,
                        .radius = 7.692307f,
                        .command_post = "cp2"},
            },
         },

      .barriers =
         {
            entities_init,
            std::initializer_list{
               barrier{.name = "Barrier0",
                       .position = {86.2013702f, 1.0f, 18.6642666f},
                       .size = {7.20497799f, 17.0095882f},
                       .rotation_angle = {2.71437049f},
                       .flags = ai_path_flags::flyer},
            },
         },

      .planning_hubs =
         {
            entities_init,
            std::initializer_list{
               planning_hub{.name = "Hub0",
                            .position = float3{-63.822487f, 0.0f, -9.202278f},
                            .radius = 8.0f},

               planning_hub{
                  .name = "Hub1",
                  .position = float3{-121.883095f, 1.0f, -30.046543f},
                  .radius = 7.586431f,
                  .weights =
                     {
                        {
                           .hub_index = 3,
                           .connection_index = 0,
                           .soldier = 20.0f,
                           .hover = 15.0f,
                           .small = 7.5f,
                           .medium = 25.0f,
                           .huge = 75.0f,
                           .flyer = 100.0f,
                        },
                     },
               },

               planning_hub{.name = "Hub2",
                            .position = float3{-54.011314f, 2.0f, -194.037018f},
                            .radius = 13.120973f},

               planning_hub{.name = "Hub3",
                            .position = float3{-163.852570f, 3.0f, -169.116760f},
                            .radius = 12.046540f}},
         },

      .planning_connections =
         {entities_init,
          std::initializer_list{
             planning_connection{.name = "Connection0",
                                 .start_hub_index = 0,
                                 .end_hub_index = 1,
                                 .flags = (ai_path_flags::soldier | ai_path_flags::hover |
                                           ai_path_flags::small | ai_path_flags::medium |
                                           ai_path_flags::huge | ai_path_flags::flyer)},

             planning_connection{
                .name = "Connection1",
                .start_hub_index = 3,
                .end_hub_index = 2,
                .flags = ai_path_flags::hover,
                .jump = true,
                .jet_jump = true,
                .one_way = true,
                .dynamic_group = 1,
             },
          }},

      .boundaries =
         {
            entities_init,
            std::initializer_list{
               boundary{.name = "boundary",
                        .points =
                           {
                              {383.557434f, 0.0f, 4.797791f},
                              {332.111023f, 0.0f, -187.202209f},
                              {191.557434f, 0.0f, -327.755798f},
                              {-0.442566f, 0.0f, -379.202209f},
                              {-192.442566f, 0.0f, -327.755798f},
                              {-332.996155f, 0.0f, -187.202209f},
                              {-384.442566f, 0.0f, 4.797791f},
                              {-332.996155f, 0.0f, 196.797791f},
                              {-192.442566f, 0.0f, 337.351379f},
                              {-0.442566f, 0.0f, 388.797791f},
                              {191.557434f, 0.0f, 337.351379f},
                              {332.111023f, 0.0f, 196.797791f},
                           }},
               boundary{.name = "boundary1",
                        .points =
                           {
                              {383.557434f, 1.0f, 4.797791f},
                              {332.111023f, 1.0f, -187.202209f},
                              {191.557434f, 1.0f, -327.755798f},
                              {-0.442566f, 1.0f, -379.202209f},
                              {-192.442566f, 1.0f, -327.755798f},
                              {-332.996155f, 1.0f, -187.202209f},
                              {-384.442566f, 1.0f, 4.797791f},
                              {-332.996155f, 1.0f, 196.797791f},
                              {-192.442566f, 1.0f, 337.351379f},
                              {-0.442566f, 1.0f, 388.797791f},
                              {191.557434f, 1.0f, 337.351379f},
                              {332.111023f, 1.0f, 196.797791f},
                           }},
            },
         },

      .measurements =
         {
            entities_init,
            std::initializer_list{
               measurement{.start = {1.0f, 0.0f, -0.0f},
                           .end = {2.0f, 0.0f, -1.0f},
                           .name = "Measurement0"},
               measurement{.start = {1.0f, 0.0f, 5.5f}, .end = {-1.0f, 0.0f, -0.0f}, .name = ""},
            },
         },

      .animations =
         {
            pinned_vector_init{max_animations, max_animations},
            std::initializer_list{
               animation{
                  .name = "Anim",
                  .runtime = 10.0f,
                  .loop = false,
                  .local_translation = true,

                  .position_keys =
                     {
                        position_key{
                           .time = 0.0f,
                           .position = float3{10.0f, 0.0f, 0.0f},
                           .transition = animation_transition::pop,
                           .tangent = float3{-0.0f, 0.0f, 0.0f},
                           .tangent_next = float3{-0.0f, 0.0f, 0.0f},
                        },
                        position_key{
                           .time = 5.0f,
                           .position = float3{50.0f, 30.0f, 78.0f},
                           .transition = animation_transition::linear,
                           .tangent = float3{-0.0f, 0.0f, 0.0f},
                           .tangent_next = float3{-0.0f, 0.0f, 0.0f},
                        },
                        position_key{
                           .time = 10.0f,
                           .position = float3{60.0f, 30.0f, 78.0f},
                           .transition = animation_transition::spline,
                           .tangent = float3{-10.0f, 0.0f, 0.0f},
                           .tangent_next = float3{-0.0f, 0.0f, 10.0f},
                        },
                     },

                  .rotation_keys =
                     {
                        rotation_key{
                           .time = 0.0f,
                           .rotation = float3{0.00f, -0.00f, -0.00f},
                           .transition = animation_transition::linear,
                           .tangent = float3{0.00f, -0.00f, -0.00f},
                           .tangent_next = float3{0.00f, -0.00f, -0.00f},
                        },
                        rotation_key{
                           .time = 5.0f,
                           .rotation = float3{0.00f, -45.00f, -0.00f},
                           .transition = animation_transition::spline,
                           .tangent = float3{35.00f, -0.00f, -0.00f},
                           .tangent_next = float3{0.00f, -0.00f, -35.00f},
                        },
                        rotation_key{
                           .time = 7.5f,
                           .rotation = float3{0.00, -90.00, -0.00},
                           .transition = animation_transition::pop,
                           .tangent = float3{0.00f, -0.00f, -0.00f},
                           .tangent_next = float3{0.00f, -0.00f, -0.00f},
                        },

                     },
               },
            },
         },

      .animation_groups = {pinned_vector_init{max_animation_groups, max_animation_groups},
                           std::initializer_list{
                              animation_group{
                                 .name = "group",
                                 .play_when_level_begins = true,
                                 .stops_when_object_is_controlled = false,
                                 .disable_hierarchies = true,
                                 .entries =
                                    {
                                       animation_group::entry{"Anim",
                                                              "com_inv_col_8"},
                                    },
                              },
                           }},

      .animation_hierarchies = {pinned_vector_init{max_animation_hierarchies,
                                                   max_animation_hierarchies},
                                std::initializer_list{
                                   animation_hierarchy{
                                      .root_object = "com_inv_col_8",
                                      .objects = {"com_item_healthrecharge"},
                                   },
                                }},
   };

   save_world("temp/world/test.wld", world, {}, save_flags{});

   const auto written_wld = io::read_file_to_string("temp/world/test.wld");

   CHECK(written_wld == expected_wld);

   const auto written_lgt = io::read_file_to_string("temp/world/test.lgt");

   CHECK(written_lgt == expected_lgt);

   const auto written_pth = io::read_file_to_string("temp/world/test.pth");

   CHECK(written_pth == expected_pth);

   const auto written_rgn = io::read_file_to_string("temp/world/test.rgn");

   CHECK(written_rgn == expected_rgn);

   const auto written_pvs = io::read_file_to_string("temp/world/test.pvs");

   CHECK(written_pvs == expected_pvs);

   const auto written_hnt = io::read_file_to_string("temp/world/test.hnt");

   CHECK(written_hnt == expected_hnt);

   const auto written_bar = io::read_file_to_string("temp/world/test.bar");

   CHECK(written_bar == expected_bar);

   const auto written_pln = io::read_file_to_string("temp/world/test.pln");

   CHECK(written_pln == expected_pln);

   const auto written_bnd = io::read_file_to_string("temp/world/test.bnd");

   CHECK(written_bnd == expected_bnd);

   const auto written_msr = io::read_file_to_string("temp/world/test.msr");

   CHECK(written_msr == expected_msr);

   const auto written_anm = io::read_file_to_string("temp/world/test.anm");

   CHECK(written_anm == expected_anm);

   const auto written_ldx = io::read_file_to_string("temp/world/test.ldx");

   CHECK(written_ldx == expected_ldx);

   const auto written_req = io::read_file_to_string("temp/world/test.req");

   CHECK(written_req == expected_req);

   const auto written_mrq =
      io::read_file_to_string("temp/world/test_conquest.mrq");

   CHECK(written_mrq == expected_mrq);
}

TEST_CASE("world saving garbage collect", "[World][IO]")
{
   (void)io::create_directory("temp/world_gc");

   const std::array<std::string_view, 5> layer_files{"lyr", "pth", "rgn", "lgt", "hnt"};

   const world world{
      .name = "test",

      .deleted_layers = {"conquest", "ctf", "sound"},
      .deleted_game_modes = {"conquest", "ctf"},
   };

   for (const auto& layer : world.deleted_layers) {
      for (const auto& file : layer_files) {
         [[maybe_unused]] io::output_file output_file{
            io::path{fmt::format("temp/world_gc/test_{}.{}", layer, file)}};
      }
   }

   // Game Modes
   {
      [[maybe_unused]] io::output_file file{"temp/world_gc/test_conquest.mrq"};

      // NB: Test that test_ctf.mrq already being gone causes no issues.
   }

   save_world("temp/world_gc/test.wld", world, {}, save_flags{});

   for (const auto& layer : world.deleted_layers) {
      for (const auto& file : layer_files) {
         CHECK(not io::exists(
            io::path{fmt::format("temp/world_gc/test_{}.{}", layer, file)}));
      }
   }

   for (const auto& game_mode : world.deleted_game_modes) {
      CHECK(not io::exists(
         io::path{fmt::format("temp/world_gc/test_{}.mrq", game_mode)}));
   }
}

TEST_CASE("world saving no gamemodes bf1 format", "[World][IO]")
{
   (void)io::create_directory("temp/world");

   const world world{
      .name = "test_no_gamemodes",

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

      .game_modes = {{.name = "conquest",
                      .layers = {1},
                      .requirements =
                         {
                            {.file_type = "world", .entries = {"test_conquest"}},
                         }}},
      .common_layers = {0},
   };

   save_world("temp/world/test_no_gamemodes.wld", world, {}, {.save_bf1_format = true});

   const auto written_ldx =
      io::read_file_to_string("temp/world/test_no_gamemodes.ldx");

   CHECK(written_ldx == expected_ldx_no_gamemodes);

   const auto written_req =
      io::read_file_to_string("temp/world/test_no_gamemodes.req");

   CHECK(written_req == expected_req);

   CHECK(not io::exists("temp/world/test_no_gamemodes_conquest.mrq"));
}

TEST_CASE("world saving boundary bf1 format", "[World][IO]")
{
   (void)io::create_directory("temp/world");

   const world world{
      .name = "test_no_boundary_spline_type",

      .layer_descriptions = {{.name = "[Base]"}},

      .boundaries =
         {
            entities_init,
            std::initializer_list{
               boundary{.name = "boundary",
                        .points =
                           {
                              {383.557434f, 0.0f, 4.797791f},
                              {332.111023f, 0.0f, -187.202209f},
                              {191.557434f, 0.0f, -327.755798f},
                              {-0.442566f, 0.0f, -379.202209f},
                              {-192.442566f, 0.0f, -327.755798f},
                              {-332.996155f, 0.0f, -187.202209f},
                              {-384.442566f, 0.0f, 4.797791f},
                              {-332.996155f, 0.0f, 196.797791f},
                              {-192.442566f, 0.0f, 337.351379f},
                              {-0.442566f, 0.0f, 388.797791f},
                              {191.557434f, 0.0f, 337.351379f},
                              {332.111023f, 0.0f, 196.797791f},
                           }},
               boundary{.name = "boundary1",
                        .points =
                           {
                              {383.557434f, 1.0f, 4.797791f},
                              {332.111023f, 1.0f, -187.202209f},
                              {191.557434f, 1.0f, -327.755798f},
                              {-0.442566f, 1.0f, -379.202209f},
                              {-192.442566f, 1.0f, -327.755798f},
                              {-332.996155f, 1.0f, -187.202209f},
                              {-384.442566f, 1.0f, 4.797791f},
                              {-332.996155f, 1.0f, 196.797791f},
                              {-192.442566f, 1.0f, 337.351379f},
                              {-0.442566f, 1.0f, 388.797791f},
                              {191.557434f, 1.0f, 337.351379f},
                              {332.111023f, 1.0f, 196.797791f},
                           }},
            },
         },
   };

   save_world("temp/world/test_no_boundary_spline_type.wld", world, {},
              {.save_bf1_format = true});

   const auto written_pth =
      io::read_file_to_string("temp/world/test_no_boundary_spline_type.pth");

   CHECK(written_pth == expected_pth_no_boundary_spline_type);
}

TEST_CASE("world saving no effects", "[World][IO]")
{
   (void)io::create_directory("temp/world");

   const world world{
      .name = "test_no_effects",

      .layer_descriptions = {{.name = "[Base]"}},
   };

   save_world("temp/world/test_no_effects.wld", world, {}, {.save_effects = false});

   CHECK(not io::exists("temp/world/test_no_effects.fx"));
}

TEST_CASE("world saving blocks req and layer", "[World][IO]")
{
   (void)io::create_directory("temp/world_blocks");

   world world{
      .name = "test_blocks",
      .requirements =
         {
            {.file_type = "path", .entries = {"test_blocks"}},
            {.file_type = "congraph", .entries = {"test_blocks"}},
            {.file_type = "envfx", .entries = {"test_blocks"}},
            {.file_type = "world", .entries = {"test_blocks"}},
            {.file_type = "prop", .entries = {"test_blocks"}},
            {.file_type = "povs", .entries = {"test_blocks"}},
            {.file_type = "lvl", .entries = {}},
         },

      .layer_descriptions = {{.name = "[Base]"}},
   };
   creation_entity creation_entity;
   edit_context edit_context{.world = world, .creation_entity = creation_entity};

   edits::make_add_block({.size = {4.0f, 4.0f, 4.0f}}, 0,
                         world.blocks.next_id.boxes.aquire())
      ->apply(edit_context);

   save_world("temp/world_blocks/test_blocks.wld", world, {},
              {.save_blocks_into_layer = true});

   constexpr auto expected_blocks_req = R"(ucft
{
	REQN
	{
		"path"
		"test_blocks"
	}
	REQN
	{
		"congraph"
		"test_blocks"
	}
	REQN
	{
		"envfx"
		"test_blocks"
	}
	REQN
	{
		"world"
		"test_blocks"
		"test_blocks_WE_blocks"
	}
	REQN
	{
		"prop"
		"test_blocks"
	}
	REQN
	{
		"povs"
		"test_blocks"
	}
	REQN
	{
		"lvl"
	}
}
)"sv;

   const auto written_req =
      io::read_file_to_string("temp/world_blocks/test_blocks.req");

   CHECK(written_req == expected_blocks_req);

   constexpr auto expected_blocks_lyr = R"(Version(3);
SaveType(0);

Object("", "WE_test_blocks_blocks0", 0)
{
	ChildRotation(0.000000, 0.000000, 1.000000, 0.000000);
	ChildPosition(0.000000, 0.000000, 0.000000);
	Team(0);
	NetworkId(-1);
}

)"sv;

   const auto written_lyr =
      io::read_file_to_string("temp/world_blocks/test_blocks_WE_blocks.lyr");

   CHECK(written_lyr == expected_blocks_lyr);
}

TEST_CASE("world saving blocks layer garbage collect", "[World][IO]")
{
   (void)io::create_directory("temp/world_blocks_gc");

   world world{
      .name = "test_blocks",
      .requirements =
         {
            {.file_type = "path", .entries = {"test_blocks"}},
            {.file_type = "congraph", .entries = {"test_blocks"}},
            {.file_type = "envfx", .entries = {"test_blocks"}},
            {.file_type = "world", .entries = {"test_blocks"}},
            {.file_type = "prop", .entries = {"test_blocks"}},
            {.file_type = "povs", .entries = {"test_blocks"}},
            {.file_type = "lvl", .entries = {}},
         },

      .layer_descriptions = {{.name = "[Base]"}},
   };
   creation_entity creation_entity;
   edit_context edit_context{.world = world, .creation_entity = creation_entity};

   edits::make_add_block({.size = {4.0f, 4.0f, 4.0f}}, 0,
                         world.blocks.next_id.boxes.aquire())
      ->apply(edit_context);

   const io::path layer_path = "temp/world_blocks_gc/test_blocks_WE_blocks.lyr";

   io::output_file{layer_path};

   save_world("temp/world_blocks_gc/test_blocks.wld", world, {},
              {.save_blocks_into_layer = false});

   CHECK(not io::exists(layer_path));
}

TEST_CASE("world saving blocks req insertion", "[World][IO]")
{
   (void)io::create_directory("temp/world_test_blocks_req");

   world world{
      .name = "test_blocks",
      .requirements =
         {
            {.file_type = "path", .entries = {"test_blocks"}},
            {.file_type = "congraph", .entries = {"test_blocks"}},
            {.file_type = "envfx", .entries = {"test_blocks"}},
            {.file_type = "world", .entries = {"test_blocks"}},
            {.file_type = "prop", .entries = {"test_blocks"}},
            {.file_type = "povs", .entries = {"test_blocks"}},
            {.file_type = "lvl", .entries = {}},
         },

      .layer_descriptions = {{.name = "[Base]"}},
   };
   creation_entity creation_entity;
   edit_context edit_context{.world = world, .creation_entity = creation_entity};

   edits::make_add_block({.size = {4.0f, 4.0f, 4.0f}}, 0,
                         world.blocks.next_id.boxes.aquire())
      ->apply(edit_context);

   save_world("temp/world_test_blocks_req/test_blocks.wld", world, {},
              {.save_blocks_into_layer = false});

   constexpr auto expected_blocks_req = R"(ucft
{
	REQN
	{
		"path"
		"test_blocks"
	}
	REQN
	{
		"congraph"
		"test_blocks"
	}
	REQN
	{
		"envfx"
		"test_blocks"
	}
	REQN
	{
		"world"
		"test_blocks"
	}
	REQN
	{
		"blocks"
		"test_blocks"
	}
	REQN
	{
		"prop"
		"test_blocks"
	}
	REQN
	{
		"povs"
		"test_blocks"
	}
	REQN
	{
		"lvl"
	}
}
)"sv;

   const auto written_req =
      io::read_file_to_string("temp/world_test_blocks_req/test_blocks.req");

   CHECK(written_req == expected_blocks_req);
}

TEST_CASE("world saving blocks req existing", "[World][IO]")
{
   (void)io::create_directory("temp/world_blocks_existing_req");

   world world{
      .name = "test_blocks",
      .requirements =
         {
            {.file_type = "path", .entries = {"test_blocks"}},
            {.file_type = "congraph", .entries = {"test_blocks"}},
            {.file_type = "envfx", .entries = {"test_blocks"}},
            {.file_type = "world", .entries = {"test_blocks"}},
            {.file_type = "blocks", .entries = {"test_blocks"}},
            {.file_type = "prop", .entries = {"test_blocks"}},
            {.file_type = "povs", .entries = {"test_blocks"}},
            {.file_type = "lvl", .entries = {}},
         },

      .layer_descriptions = {{.name = "[Base]"}},
   };
   creation_entity creation_entity;
   edit_context edit_context{.world = world, .creation_entity = creation_entity};

   edits::make_add_block({.size = {4.0f, 4.0f, 4.0f}}, 0,
                         world.blocks.next_id.boxes.aquire())
      ->apply(edit_context);

   save_world("temp/world_blocks_existing_req/test_blocks.wld", world, {},
              {.save_blocks_into_layer = false});

   constexpr auto expected_blocks_req = R"(ucft
{
	REQN
	{
		"path"
		"test_blocks"
	}
	REQN
	{
		"congraph"
		"test_blocks"
	}
	REQN
	{
		"envfx"
		"test_blocks"
	}
	REQN
	{
		"world"
		"test_blocks"
	}
	REQN
	{
		"blocks"
		"test_blocks"
	}
	REQN
	{
		"prop"
		"test_blocks"
	}
	REQN
	{
		"povs"
		"test_blocks"
	}
	REQN
	{
		"lvl"
	}
}
)"sv;

   const auto written_req =
      io::read_file_to_string("temp/world_blocks_existing_req/test_blocks.req");

   CHECK(written_req == expected_blocks_req);
}

}
