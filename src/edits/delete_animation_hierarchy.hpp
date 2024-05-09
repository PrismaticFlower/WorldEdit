#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_animation_hierarchy(uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>;

}
