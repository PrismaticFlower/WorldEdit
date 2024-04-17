#pragma once

#include "edit.hpp"
#include "types.hpp"
#include "world/interaction_context.hpp"

#include <memory>
#include <string>

namespace we::edits {

auto make_rename_game_mode(uint32 index, std::string new_name, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}
