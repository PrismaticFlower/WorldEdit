#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_block(world::block_description_cube box)
   -> std::unique_ptr<edit<world::edit_context>>;

}
