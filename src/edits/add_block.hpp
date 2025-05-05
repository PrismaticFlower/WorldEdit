#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_add_block(world::block_description_box box, int8 layer, world::block_box_id id)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_add_block(world::block_description_ramp ramp, int8 layer,
                    world::block_ramp_id id)
   -> std::unique_ptr<edit<world::edit_context>>;

}
