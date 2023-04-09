#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_world_req_list(std::string file_type)
   -> std::unique_ptr<edit<world::edit_context>>;

}
