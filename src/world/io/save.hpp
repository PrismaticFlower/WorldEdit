#pragma once

#include "../world.hpp"
#include "io/path.hpp"
#include "output_stream.hpp"

#include <span>

namespace we::world {

struct save_flags {
   /// @brief Save into BF1 compatible files. (Excluding terrain, it has a separate property).
   ///
   /// Below is a list of most changes this flag causes.
   ///
   /// - Exclude PathType and saving the SplineType property in boundary paths.
   ///
   /// - Exclude GameMode section in the layer index.
   ///
   /// - Skip the saving of .mrq files.
   bool save_bf1_format : 1 = false;

   /// @brief Controls saving the .fx file.
   bool save_effects : 1 = true;

   /// @brief Save blocks layer.
   bool save_blocks_into_layer : 1 = false;
};

void save_world(const io::path& path, const world& world,
                const std::span<const terrain_cut> terrain_cuts,
                const save_flags flags);

}
