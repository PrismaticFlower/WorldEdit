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

auto make_add_block(world::block_description_quad quad, int8 layer,
                    world::block_quad_id id)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_add_block(world::block_description_cylinder cylinder, int8 layer,
                    world::block_cylinder_id id)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_add_block(world::block_description_stairway stairway, int8 layer,
                    world::block_stairway_id id)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_add_block(world::block_description_cone cone, int8 layer,
                    world::block_cone_id id)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_add_block(world::block_description_hemisphere hemisphere, int8 layer,
                    world::block_hemisphere_id id)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_add_block(world::block_description_pyramid pyramid, int8 layer,
                    world::block_pyramid_id id)
   -> std::unique_ptr<edit<world::edit_context>>;
}
