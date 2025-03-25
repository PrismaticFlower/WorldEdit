#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_set_block_box_metrics(const uint32 index, const quaternion& rotation,
                                const float3& position, const float3& size) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_block_box_surface(const uint32 index, const uint32 surface_index,
                                const world::block_texture_rotation rotation) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

}
