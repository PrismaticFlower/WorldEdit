#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_set_block_cube_metrics(const uint32 index, const quaternion& rotation,
                                 const float3& position, const float3& size) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

}
