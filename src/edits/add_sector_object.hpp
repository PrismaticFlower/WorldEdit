#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_sector_object(world::sector_id sector_id, std::string object_name)
   -> std::unique_ptr<edit<world::edit_context>>;

}
