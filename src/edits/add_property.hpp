#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_property(world::path_id path_id, std::string property)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_add_property(world::path_id path_id, std::size_t node, std::string property)
   -> std::unique_ptr<edit<world::edit_context>>;

}
