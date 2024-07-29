#include "pch.h"

#include "edits/insert_entity.hpp"
#include "world/object_class_library.hpp"
#include "world/world.hpp"

#include "null_asset_libraries.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits insert_entity", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::boundary boundary{.name = "Boundary2", .id = world::boundary_id{1}};

   auto action = make_insert_entity(boundary);

   action->apply(edit_context);

   REQUIRE(world.boundaries.size() == 2);
   REQUIRE(world.boundaries[1].name == "Boundary2");

   action->revert(edit_context);

   REQUIRE(world.boundaries.size() == 1);
   REQUIRE(world.boundaries[0].name != "Boundary2");
}

TEST_CASE("edits add_object", "[Edits]")
{
   world::world world = {};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   world::object object{.name = "wall",
                        .class_name = lowercase_string{"test_bldg_icewall"sv},
                        .id = world::object_id{1}};

   auto edit = make_insert_entity(object, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.objects.size() == 1);
   CHECK(world.objects[0].name == "wall");
   CHECK(world.objects[0].class_name == "test_bldg_icewall");

   CHECK(object_class_library.debug_ref_count(object.class_name) == 1);

   edit->revert(edit_context);

   REQUIRE(world.objects.empty());

   CHECK(object_class_library.debug_ref_count(object.class_name) == 0);
}

}
