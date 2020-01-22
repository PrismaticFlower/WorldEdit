
Light("Light 2", 1413102100)
{
	Rotation(0.998519, 0.000000, 0.000000, -0.054843);
	Position(-128.463806, 0.855094, -22.575970);
	Type(2);
	Color(0.501961, 0.376471, 0.376471);
	Static();
	CastSpecular(1);
	Range(5.000000);
}

Light("sun", 2793467422)
{
	Rotation(0.922373, 0.384204, -0.039542, -0.008615);
	Position(-159.264923, 19.331013, -66.727310);
	Type(1);
	Color(1.000000, 0.882353, 0.752941);
	CastShadow();
	Static();
	CastSpecular(1);
	PS2BlendMode(0);
	TileUV(1.000000,1.000000);
	OffsetUV(0.000000,0.000000);
}

Light("Light 3", 3551084123)
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
	Bidirectional(0);
}

Light("Light 1", 4266109616)
{
	Rotation(0.924904, 0.000000, 0.380202, 0.000000);
	Position(-129.618546, 5.019108, -27.300539);
	Type(2);
	Color(0.498039, 0.498039, 0.627451);
	Static();
	CastSpecular(1);
	Range(16.000000);
}

GlobalLights()
{
	Light1("sun");
	Light2("");
	Top(140, 79, 63);
	Bottom(80, 40, 30);
}
