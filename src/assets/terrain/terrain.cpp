#include "terrain.hpp"

namespace we::assets::terrain {

void terrain::untracked_fill_dirty_rects() noexcept
{
   const uint32 length_u32 =
      static_cast<uint32>(length); // TODO: Change terrain.length to uint32.

   height_map_dirty.add({0, 0, length_u32, length_u32});

   texture_weight_maps_dirty.add({0, 0, length_u32, length_u32});

   color_or_light_map_dirty.add({0, 0, length_u32, length_u32});

   water_map_dirty.add({0, 0, length_u32 / 4u, length_u32 / 4u});

   foliage_map_dirty.add({0, 0, length_u32 / 2u, length_u32 / 2u});
}

void terrain::untracked_clear_dirty_rects() noexcept
{
   height_map_dirty.clear();

   texture_weight_maps_dirty.clear();

   color_or_light_map_dirty.clear();

   water_map_dirty.clear();

   foliage_map_dirty.clear();
}

}