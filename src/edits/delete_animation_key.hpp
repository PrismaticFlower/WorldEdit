#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_animation_key(std::vector<world::position_key>* keys, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_animation_key(std::vector<world::rotation_key>* keys, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>;

}
