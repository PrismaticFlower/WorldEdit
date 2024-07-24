#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_branch_weight(std::vector<world::planning_branch_weights>* branch_weights,
                               uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>;

}
