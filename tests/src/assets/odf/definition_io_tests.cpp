#include "pch.h"

#include "assets/odf/definition_io.hpp"

#include <string_view>

using namespace std::literals;

namespace we::assets::odf::tests {

namespace {

const std::string_view valid_odf_test = R"(// First line is ignored.
[GameObjectClass] // Comment after [GameObjectClass]

// This is a test .odf
\\ Line comments can also begin with '\\'
-- Another comment

ClassLabel = "prop"
GeometryName = "test_prop_sphere.msh"

[Properties]

GeometryName = test_prop_sphere
QuotedProp = "quoted values can have spaces //this is part of the value " // Comment
SemiquotedProp = "quoted values do not need an ending quote // this is part of the value 

[InstanceProperties]

InstanceValue = 1.8)";

const std::string_view valid_odf_test_mixed_line_breaks =
   "[GameObjectClass] // Comment after [GameObjectClass]\n"
   "ClassLabel = \"prop\"\r\n"
   "GeometryName = \"test_prop_sphere.msh\"\n"
   "[Properties]\n"
   "SemiquotedProp = \"quoted values do not need an ending quote // this is "
   "part of the value \r\n";

const std::string_view valid_odf_test_class_parent = R"([GameObjectClass] 

ClassParent = "test_prop_water"
GeometryName = "test_prop_plane.msh"

[Properties]

GeometryName = test_prop_plane)";

const std::string_view valid_odf_test_explosion_class = R"([ExplosionClass] 

ClassName = "explosion"

[Properties]
)";

const std::string_view valid_odf_test_ordnance_class = R"([OrdnanceClass] 

ClassName = "bullet"

[Properties]
)";

const std::string_view valid_odf_test_weapon_class = R"([WeaponClass] 

ClassName = "cannon"

[Properties]
)";

const std::string_view invalid_odf_test_bad_header_name =
   R"([FameObjectGlass] // Line #1 Expect Error Here

ClassLabel = "prop"
GeometryName = "test_prop_sphere.msh" 

[Properties])";

const std::string_view invalid_odf_test_bad_key_value = R"([GameObjectClass]

ClassLabel = "prop" // Expect error the on below line.
GeometryName "test_prop_sphere.msh"

[Properties])";

const std::string_view invalid_odf_test_empty_value = R"([GameObjectClass]

ClassLabel = "prop" // Expect error the on below line.
GeometryName = 

[Properties])";

const std::string_view invalid_odf_test_no_assignment = R"([GameObjectClass]

ClassLabel = "prop" // Expect error the on below line.
GeometryName

[Properties])";

auto to_vector(std::string_view string) -> std::vector<char>
{
   return {string.begin(), string.end()};
}

}

TEST_CASE(".odf reading", "[Assets][ODF]")
{
   auto definition = read_definition(to_vector(valid_odf_test));

   REQUIRE(definition.type == type::game_object_class);

   CHECK(definition.header.base == "prop"sv);
   CHECK(definition.header.geometry_name == "test_prop_sphere.msh"sv);

   CHECK(definition.properties["GeometryName"sv] == "test_prop_sphere"sv);
   CHECK(definition.properties["QuotedProp"sv] ==
         "quoted values can have spaces //this is part of the value "sv);
   CHECK(definition.properties["SemiquotedProp"sv] ==
         "quoted values do not need an ending quote // this is part of the value "sv);

   CHECK(definition.instance_properties["InstanceValue"sv] == "1.8"sv);
}

TEST_CASE(".odf reading mixed line breaks", "[Assets][ODF]")
{
   auto definition = read_definition(to_vector(valid_odf_test_mixed_line_breaks));

   REQUIRE(definition.type == type::game_object_class);

   CHECK(definition.header.base == "prop"sv);
   CHECK(definition.header.geometry_name == "test_prop_sphere.msh"sv);
   CHECK(definition.properties["SemiquotedProp"sv] ==
         "quoted values do not need an ending quote // this is "
         "part of the value "sv);
}

TEST_CASE(".odf reading class parent", "[Assets][ODF]")
{
   auto definition = read_definition(to_vector(valid_odf_test_class_parent));

   REQUIRE(definition.type == type::game_object_class);

   CHECK(definition.header.base == "test_prop_water"sv);
   CHECK(definition.header.geometry_name == "test_prop_plane.msh"sv);
}

TEST_CASE(".odf reading explosion class", "[Assets][ODF]")
{
   auto definition = read_definition(to_vector(valid_odf_test_explosion_class));

   REQUIRE(definition.type == type::explosion_class);
}

TEST_CASE(".odf reading ordnance class", "[Assets][ODF]")
{
   auto definition = read_definition(to_vector(valid_odf_test_ordnance_class));

   REQUIRE(definition.type == type::ordnance_class);
}

TEST_CASE(".odf reading weapon class", "[Assets][ODF]")
{
   auto definition = read_definition(to_vector(valid_odf_test_weapon_class));

   REQUIRE(definition.type == type::weapon_class);
}

TEST_CASE(".odf failed reading bad header", "[Assets][ODF]")
{
   REQUIRE_THROWS(read_definition(to_vector(invalid_odf_test_bad_header_name)));
}

TEST_CASE(".odf failed reading bad key value", "[Assets][ODF]")
{
   REQUIRE_THROWS(read_definition(to_vector(invalid_odf_test_bad_key_value)));
}

TEST_CASE(".odf failed reading empty value", "[Assets][ODF]")
{
   REQUIRE_THROWS(read_definition(to_vector(invalid_odf_test_empty_value)));
}

TEST_CASE(".odf failed reading no assignment", "[Assets][ODF]")
{
   REQUIRE_THROWS(read_definition(to_vector(invalid_odf_test_no_assignment)));
}
}
