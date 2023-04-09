#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_world_req_entry(int list_index, std::string name)
   -> std::unique_ptr<edit<world::edit_context>>;

}
