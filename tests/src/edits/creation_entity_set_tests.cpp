#include "pch.h"

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "edits/creation_entity_set.hpp"
#include "output_stream.hpp"
#include "world/object_class_library.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits creation_entity_set", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};
   world::object_class_library library{assets_libraries};

   world::light light{.name = "Stars"};

   auto action = make_creation_entity_set(light, library);

   action->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.is<world::light>());
   CHECK(interaction_targets.creation_entity.get<world::light>() == light);

   action->revert(edit_context);

   CHECK(not interaction_targets.creation_entity.holds_entity());
}

TEST_CASE(
   "edits creation_entity_set object_class_handle lifetime none to object",
   "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};
   world::object_class_library object_class_library{assets_libraries};

   const lowercase_string map_prop_stars{"map_prop_stars"sv};

   world::object object{.name = "Stars", .class_name = map_prop_stars};

   auto action = make_creation_entity_set(object, object_class_library);

   action->apply(edit_context);

   CHECK(object_class_library.debug_ref_count(map_prop_stars) == 1);

   action->revert(edit_context);

   CHECK(object_class_library.debug_ref_count(map_prop_stars) == 0);
}

TEST_CASE(
   "edits creation_entity_set object_class_handle lifetime object to object", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};
   world::object_class_library object_class_library{assets_libraries};

   const lowercase_string class_name_stars{"map_prop_stars"sv};
   const lowercase_string class_name_moon{"map_prop_moon"sv};

   interaction_targets.creation_entity =
      world::object{.name = "Stars",
                    .class_handle = object_class_library.acquire(class_name_stars),
                    .class_name = class_name_stars};

   world::object object{.name = "Moon", .class_name = class_name_moon};

   auto action = make_creation_entity_set(object, object_class_library);

   action->apply(edit_context);

   CHECK(object_class_library.debug_ref_count(class_name_stars) == 0);
   CHECK(object_class_library.debug_ref_count(class_name_moon) == 1);

   action->revert(edit_context);

   CHECK(object_class_library.debug_ref_count(class_name_stars) == 1);
   CHECK(object_class_library.debug_ref_count(class_name_moon) == 0);
}

TEST_CASE(
   "edits creation_entity_set object_class_handle lifetime light to object",
   "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};
   world::object_class_library object_class_library{assets_libraries};

   const lowercase_string class_name_stars{"map_prop_stars"sv};

   interaction_targets.creation_entity = world::light{.name = "Sun"};

   world::object object{.name = "Stars", .class_name = class_name_stars};

   auto action = make_creation_entity_set(object, object_class_library);

   action->apply(edit_context);

   CHECK(object_class_library.debug_ref_count(class_name_stars) == 1);

   action->revert(edit_context);

   CHECK(object_class_library.debug_ref_count(class_name_stars) == 0);
}

TEST_CASE(
   "edits creation_entity_set object_class_handle lifetime object to light",
   "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};
   world::object_class_library object_class_library{assets_libraries};

   const lowercase_string class_name_stars{"map_prop_stars"sv};

   interaction_targets.creation_entity =
      world::object{.name = "Stars",
                    .class_handle = object_class_library.acquire(class_name_stars),
                    .class_name = class_name_stars};

   world::light light{.name = "Moon"};

   auto action = make_creation_entity_set(light, object_class_library);

   action->apply(edit_context);

   CHECK(object_class_library.debug_ref_count(class_name_stars) == 0);

   action->revert(edit_context);

   CHECK(object_class_library.debug_ref_count(class_name_stars) == 1);
}

}
