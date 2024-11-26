#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_game_mode_link_layer(int game_mode_index, int layer_index,
                               const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_game_mode_link_common_layer(int layer_index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}
