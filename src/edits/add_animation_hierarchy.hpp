#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_animation_hierarchy(world::animation_hierarchy hierarchy)
   -> std::unique_ptr<edit<world::edit_context>>;

}
