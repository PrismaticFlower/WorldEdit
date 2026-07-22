#include "pch.h"

#include "edits/add_tree_line.hpp"

#include "world/object_class_library.hpp"
#include "world/world.hpp"

#include "null_asset_libraries.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits add_tree_line", "[Edits]")
{
   world::world world = {};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   world::tree_line tree_line = {
      .border_odfs = {{"test_bldg_icewall"}},
      .path_index = 0,
   };

   auto edit = make_add_tree_line(tree_line, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.tree_lines.size() == 1);
   REQUIRE(world.tree_lines[0].border_odfs.size() == 1);

   CHECK(world.tree_lines[0].border_odfs[0].name == "test_bldg_icewall");

   CHECK(object_class_library.debug_ref_count(
            lowercase_string{tree_line.border_odfs[0].name}) == 1);

   edit->revert(edit_context);

   REQUIRE(world.tree_lines.empty());

   CHECK(object_class_library.debug_ref_count(
            lowercase_string{tree_line.border_odfs[0].name}) == 0);
}

}
