#include "pch.h"

#include "edits/set_class_name.hpp"
#include "world/object_class_library.hpp"
#include "world/world.hpp"

#include "null_asset_libraries.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits set_class_name", "[Edits]")
{
   world::world world = test_world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::object_class_library object_class_library{null_asset_libraries()};

   const lowercase_string old_class_name{"test_prop_bush"sv};
   const lowercase_string new_class_name{"test_prop_rock"sv};

   world.objects[0].class_name = old_class_name;
   world.objects[0].class_handle = object_class_library.acquire(old_class_name);

   auto edit =
      make_set_class_name(&world.objects[0], new_class_name, object_class_library);

   edit->apply(edit_context);

   CHECK(world.objects[0].class_name == new_class_name);
   CHECK(object_class_library.debug_ref_count(old_class_name) == 0);
   CHECK(object_class_library.debug_ref_count(new_class_name) == 1);

   edit->revert(edit_context);

   CHECK(world.objects[0].class_name == old_class_name);
   CHECK(object_class_library.debug_ref_count(old_class_name) == 1);
   CHECK(object_class_library.debug_ref_count(new_class_name) == 0);
}

}
