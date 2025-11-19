#pragma once

#include "edit.hpp"

#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_sun_flare(uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>;

}
