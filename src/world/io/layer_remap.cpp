#include "layer_remap.hpp"

namespace we::world {

void layer_remap::set(int old_index, int8 new_index) noexcept
{
   if (old_index < 0 or old_index >= max_layers) return;
   if (new_index < 0 or new_index >= max_layers) return;

   _map[old_index] = new_index;
}

int8 layer_remap::operator[](int layer) const noexcept
{
   if (layer < 0 or layer >= max_layers) return 0;

   return _map[layer];
}

}