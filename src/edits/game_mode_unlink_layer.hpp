#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_game_mode_unlink_layer(int game_mode_index, int game_mode_layers_index,
                                 const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_game_mode_unlink_common_layer(int common_layers_index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}
