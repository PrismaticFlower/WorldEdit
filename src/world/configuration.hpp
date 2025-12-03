#pragma once

namespace we::world {

struct configuration {
   /// @brief Save into BF1 compatible files. (Excluding terrain, it has a separate property).
   ///
   /// Below is a list of most changes this flag causes.
   ///
   /// - Exclude PathType and saving the SplineType property in boundary paths.
   ///
   /// - Exclude GameMode section in the layer index.
   ///
   /// - Skip the saving of .mrq files.
   bool save_bf1_format = false;

   /// @brief Controls saving the .fx file.
   bool save_effects = true;

   /// @brief Save blocks layer.
   bool save_blocks_into_layer = true;

   /// @brief Save LightName in .wld/.lyr files.
   bool save_lights_references = true;

   /// @brief Save SkyName in the .wld file.
   bool save_sky_reference = true;
};

}