#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_set_world_req_entry(uint32 list_index, uint32 entry_index, std::string value)
   -> std::unique_ptr<edit<world::edit_context>>;

}
