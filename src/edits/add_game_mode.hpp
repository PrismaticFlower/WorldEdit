#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_game_mode(std::string name, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}
