#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_creation_entity_set(world::creation_entity new_creation_entity)
   -> std::unique_ptr<edit<world::edit_context>>;

}
