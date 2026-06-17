#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_animation_hierarchy_child(std::vector<uint32>* children, uint32 new_child)
   -> std::unique_ptr<edit<world::edit_context>>;

}
