#include "pch.h"

#include "edits/add_tree_line_border_odf.hpp"

#include "world/object_class_library.hpp"
#include "world/world.hpp"

#include "null_asset_libraries.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_tree_line_border_odf", "[Edits]")
{
   world::world world = {};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   world.tree_lines.push_back(world::tree_line{});

   world::tree_line_odf odf = {.name = "test_bldg_icewall"};

   auto edit = make_add_tree_line_border_odf(&world.tree_lines[0].border_odfs,
                                             odf, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.tree_lines[0].border_odfs.size() == 1);

   CHECK(world.tree_lines[0].border_odfs[0].name == "test_bldg_icewall");

   CHECK(object_class_library.debug_ref_count(lowercase_string{odf.name}) == 1);

   edit->revert(edit_context);

   REQUIRE(world.tree_lines[0].border_odfs.empty());

   CHECK(object_class_library.debug_ref_count(lowercase_string{odf.name}) == 0);
}

}
