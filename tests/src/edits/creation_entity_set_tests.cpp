#include "pch.h"

#include "edits/creation_entity_set.hpp"
#include "world/world.hpp"

using namespace std::literals;

namespace we::edits::tests {

TEST_CASE("edits creation_entity_set", "[Edits]")
{
   world::world world;
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   world::object object{.name = "Stars"};

   auto action = make_creation_entity_set(object);

   action->apply(edit_context);

   REQUIRE(interaction_targets.creation_entity.is<world::object>());
   CHECK(interaction_targets.creation_entity.get<world::object>() == object);

   action->revert(edit_context);

   CHECK(not interaction_targets.creation_entity.holds_entity());
}

}
