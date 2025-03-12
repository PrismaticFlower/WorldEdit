#pragma once

#include "id.hpp"
#include "types.hpp"

#include "blocks/dirty_range_tracker.hpp"

#include "container/pinned_vector.hpp"
#include "container/slim_bitset.hpp"

namespace we::world {

constexpr std::size_t max_blocks = 1'048'576;
constexpr std::size_t reserved_blocks = 2048;
constexpr pinned_vector_init blocks_init{.max_size = max_blocks,
                                         .initial_capacity = reserved_blocks};

struct block_description_cube {
   quaternion rotation;
   float3 position;
   float3 size;
};

struct blocks_bbox_soa {
   pinned_vector<float> min_x = blocks_init;
   pinned_vector<float> min_y = blocks_init;
   pinned_vector<float> min_z = blocks_init;
   pinned_vector<float> max_x = blocks_init;
   pinned_vector<float> max_y = blocks_init;
   pinned_vector<float> max_z = blocks_init;
};

struct blocks_cubes {
   blocks_bbox_soa bbox;

   pinned_vector<bool> hidden = blocks_init;

   pinned_vector<block_description_cube> description = blocks_init;

   pinned_vector<id<block_description_cube>> ids = blocks_init;

   blocks_dirty_range_tracker dirty;

   auto size() const noexcept -> std::size_t
   {
      return bbox.min_x.size();
   }
};

struct blocks {
   blocks_cubes cubes;

   struct next_ids {
      id_generator<block_description_cube> cubes;
   } next_id;
};

using block_cube_id = id<block_description_cube>;

}
