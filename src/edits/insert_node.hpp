#pragma once

#include "edit.hpp"
#include "world/world.hpp"

#include <memory>

namespace we::edits {

auto make_insert_node(world::path_id path_id, std::size_t insert_before_index,
                      world::path::node node) -> std::unique_ptr<edit<world::world>>;

}
