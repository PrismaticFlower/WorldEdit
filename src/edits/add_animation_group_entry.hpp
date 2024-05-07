#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_animation_group_entry(std::vector<world::animation_group::entry>* entries,
                                    world::animation_group::entry new_entry)
   -> std::unique_ptr<edit<world::edit_context>>;

}
