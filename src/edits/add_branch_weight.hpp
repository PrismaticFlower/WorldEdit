#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_branch_weight(std::vector<world::planning_branch_weights>* branch_weights,
                            world::planning_branch_weights weights)
   -> std::unique_ptr<edit<world::edit_context>>;

}
