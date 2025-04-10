#pragma once

#include "types.hpp"

#include "../layer_description.hpp"

namespace we::world {

/// @brief Remaps layers loaded from a world file to be linear. Missing layers will be assigned 0.
struct layer_remap {
   /// @brief Set the mapping for a layer.
   /// @param old_index The old layer index. Must be between [0, max_layers) else this does nothing.
   /// @param new_index The new layer index. Must be between [0, max_layers) else this does nothing.
   void set(int old_index, int8 new_index) noexcept;

   /// @brief Get the mapping for a layer.
   /// @param layer The original layer index to remap. Returns 0 if outside the range [0, max_layers).
   /// @return The new layer index.
   int8 operator[](int layer) const noexcept;

private:
   int8 _map[max_layers] = {};
};

}
