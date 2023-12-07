#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"
#include "world/terrain.hpp"

#include <memory>

namespace we::edits {

auto make_set_terrain(world::terrain new_terrain)
   -> std::unique_ptr<edit<world::edit_context>>;

}
