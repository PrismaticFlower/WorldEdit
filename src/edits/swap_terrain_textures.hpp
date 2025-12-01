#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_swap_terrain_textures(uint32 left, uint32 right)
   -> std::unique_ptr<edit<world::edit_context>>;

}
