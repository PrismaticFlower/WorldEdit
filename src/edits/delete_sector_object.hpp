#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_sector_object(world::sector_id sector_id, const std::size_t object_index,
                               const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}