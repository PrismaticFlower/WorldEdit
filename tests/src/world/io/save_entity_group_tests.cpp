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

}