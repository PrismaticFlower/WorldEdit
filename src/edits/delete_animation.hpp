#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_animation(uint32 index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}
