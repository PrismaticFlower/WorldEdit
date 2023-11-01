#include "terrain.hpp"

namespace we::assets::terrain {

void terrain::untracked_clear_dirty_rects() noexcept
{
   height_map_dirty.clear();

   for (dirty_rect_tracker& dirty : texture_weight_maps_dirty) {
      dirty.clear();
   }
}

}