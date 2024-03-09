#include "terrain.hpp"

namespace we::assets::terrain {

void terrain::untracked_fill_dirty_rects() noexcept
{
   const uint32 length_u32 =
      static_cast<uint32>(length); // TODO: Change terrain.length to uint32.

   height_map_dirty.add({0, 0, length_u32, length_u32});

   for (auto& texture_weight_map_dirty : texture_weight_maps_dirty) {
      texture_weight_map_dirty.add({0, 0, length_u32, length_u32});
   }

   color_map_dirty.add({0, 0, length_u32, length_u32});

   water_map_dirty.add({0, 0, length_u32 / 4u, length_u32 / 4u});

   foliage_map_dirty.add({0, 0, length_u32 / 2u, length_u32 / 2u});
}

void terrain::untracked_clear_dirty_rects() noexcept
{
   height_map_dirty.clear();

   for (dirty_rect_tracker& dirty : texture_weight_maps_dirty) {
      dirty.clear();
   }

   color_map_dirty.clear();

   water_map_dirty.clear();

   foliage_map_dirty.clear();
}

}