#include "pch.h"

#include "edits/delete_tree_line_border_odf.hpp"

#include "world/object_class_library.hpp"
#include "world/world.hpp"

#include "null_asset_libraries.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits delete_tree_line_border_odf", "[Edits]")
{
   world::object_class_library object_class_library{null_asset_libraries()};
   world::world world = {

      .tree_lines = {pinned_vector_init{.max_size = world::max_tree_lines},
                     std::initializer_list{world::tree_line{
                        .border_odfs = {{"test_bldg_icewall",
                                         object_class_library.acquire(lowercase_string{
                                            "test_bldg_icewall"sv})}},
                        .path_index = 0,
                     }}}

   };
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world.tree_lines.push_back(world::tree_line{});

   auto edit = make_delete_tree_line_border_odf(&world.tree_lines[0].border_odfs,
                                                0, object_class_library);

   edit->apply(edit_context);

   REQUIRE(world.tree_lines[0].border_odfs.empty());

   CHECK(object_class_library.debug_ref_count(
            lowercase_string{"test_bldg_icewall"sv}) == 0);

   edit->revert(edit_context);

   REQUIRE(world.tree_lines[0].border_odfs.size() == 1);

   CHECK(world.tree_lines[0].border_odfs[0].name == "test_bldg_icewall");

   CHECK(object_class_library.debug_ref_count(
            lowercase_string{"test_bldg_icewall"sv}) == 1);
}

}
