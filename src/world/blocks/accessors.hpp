#pragma once

#include "../blocks.hpp"

namespace we::world {

auto get_dirty_tracker(blocks& blocks, const block_type type) noexcept
   -> blocks_dirty_range_tracker&;

auto get_block_hidden(blocks& blocks, const block_type type,
                      const uint32 block_index) noexcept -> bool&;

auto get_block_layer(blocks& blocks, const block_type type,
                     const uint32 block_index) noexcept -> int8&;

auto get_block_surface_material(blocks& blocks, const block_type type,
                                const uint32 block_index,
                                const uint32 surface_index) noexcept -> uint8&;

auto get_block_surface_texture_mode(blocks& blocks, const block_type type,
                                    const uint32 block_index,
                                    const uint32 surface_index) noexcept
   -> block_texture_mode&;

auto get_block_surface_texture_rotation(blocks& blocks, const block_type type,
                                        const uint32 block_index,
                                        const uint32 surface_index) noexcept
   -> block_texture_rotation&;

auto get_block_surface_texture_scale(blocks& blocks, const block_type type,
                                     const uint32 block_index,
                                     const uint32 surface_index) noexcept
   -> std::array<int8, 2>&;

auto get_block_surface_texture_offset(blocks& blocks, const block_type type,
                                      const uint32 block_index,
                                      const uint32 surface_index) noexcept
   -> std::array<uint16, 2>&;

}
