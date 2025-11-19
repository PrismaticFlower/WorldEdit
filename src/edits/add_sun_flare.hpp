#pragma once

#include "edit.hpp"

#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_sun_flare(const world::sun_flare& sun_flare)
   -> std::unique_ptr<edit<world::edit_context>>;

}
