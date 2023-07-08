#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_path_property(world::path_id path_id, const std::size_t property_index,
                               const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_path_node_property(world::path_id path_id, const std::size_t node_index,
                                    const std::size_t property_index,
                                    const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}