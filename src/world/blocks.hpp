#pragma once

#include "id.hpp"
#include "types.hpp"

#include "blocks/dirty_range_tracker.hpp"

#include "container/pinned_vector.hpp"
#include "container/slim_bitset.hpp"

#include <array>

namespace we::world {

constexpr std::size_t max_blocks = 1'048'576;
constexpr std::size_t reserved_blocks = 2048;
constexpr pinned_vector_init blocks_init{.max_size = max_blocks,
                                         .initial_capacity = reserved_blocks};

enum class block_texture_rotation : uint8 { d0, d90, d180, d270 };

struct block_description_box {
   quaternion rotation;
   float3 position;
   float3 size;
   std::array<uint8, 6> surface_materials = {};
   std::array<block_texture_rotation, 6> surface_texture_rotation = {};
   std::array<uint8, 6> surface_texture_scale = {1, 1, 1, 1};
};

struct blocks_bbox_soa {
   pinned_vector<float> min_x = blocks_init;
   pinned_vector<float> min_y = blocks_init;
   pinned_vector<float> min_z = blocks_init;
   pinned_vector<float> max_x = blocks_init;
   pinned_vector<float> max_y = blocks_init;
   pinned_vector<float> max_z = blocks_init;
};

struct blocks_boxes {
   blocks_bbox_soa bbox;

   pinned_vector<bool> hidden = blocks_init;

   pinned_vector<block_description_box> description = blocks_init;

   pinned_vector<id<block_description_box>> ids = blocks_init;

   blocks_dirty_range_tracker dirty;

   auto size() const noexcept -> std::size_t
   {
      return bbox.min_x.size();
   }
};

struct blocks {
   blocks_boxes boxes;

   struct next_ids {
      id_generator<block_description_box> boxes;
   } next_id;
};

using block_box_id = id<block_description_box>;

}
