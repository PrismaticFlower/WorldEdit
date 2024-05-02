#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_insert_animation_key(std::vector<world::position_key>* keys,
                               std::size_t insert_before_index, world::position_key key)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_animation_key(std::vector<world::rotation_key>* keys,
                               std::size_t insert_before_index, world::rotation_key key)
   -> std::unique_ptr<edit<world::edit_context>>;

}
