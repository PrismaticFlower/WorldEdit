#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_creation_entity_set(std::optional<world::creation_entity> new_creation_entity,
                              std::optional<world::creation_entity> old_creation_entity)
   -> std::unique_ptr<edit<world::edit_context>>;

}
