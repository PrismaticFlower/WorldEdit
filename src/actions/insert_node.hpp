#pragma once

#include "action.hpp"
#include "world/world.hpp"

#include <memory>

namespace we::actions {

auto make_insert_node(world::path_id path_id, std::size_t insert_before_index,
                      world::path::node node) -> std::unique_ptr<action>;

}
