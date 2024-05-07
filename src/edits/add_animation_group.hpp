#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_animation_group(world::animation_group group)
   -> std::unique_ptr<edit<world::edit_context>>;

}
