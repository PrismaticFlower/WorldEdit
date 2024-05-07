#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_animation_group_entry(std::vector<world::animation_group::entry>* entries,
                                       uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>;

}
