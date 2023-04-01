#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_game_mode(int game_mode_index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}
