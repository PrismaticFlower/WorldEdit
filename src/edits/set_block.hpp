#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_set_block_box_metrics(const uint32 index, const quaternion& rotation,
                                const float3& position, const float3& size) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_block_surface(uint8* material_index_address,
                            uint8 new_material_index, const uint32 index,
                            world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_block_surface(world::block_texture_rotation* rotation_address,
                            world::block_texture_rotation new_rotation,
                            const uint32 index,
                            world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_block_surface(std::array<int8, 2>* scale_address,
                            std::array<int8, 2> new_scale, const uint32 index,
                            world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_block_material(std::string* texture_address,
                             std::string new_texture, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_block_material(std::array<uint8, 2>* tiling_address,
                             std::array<uint8, 2> new_tiling, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_block_material(bool* flag_address, bool new_flag, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_block_material(float3* color_address, float3 new_color, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>;
}
