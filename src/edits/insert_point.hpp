#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_insert_point(world::sector_id sector_id, std::size_t insert_before_index,
                       float2 point) -> std::unique_ptr<edit<world::edit_context>>;

}
