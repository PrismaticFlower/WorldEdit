#pragma once

#include "assets/terrain/terrain.hpp"
#include "container/dynamic_array_2d.hpp"
#include "types.hpp"

namespace sk::world {

using assets::terrain::active_flags;
using assets::terrain::foliage_patch;
using assets::terrain::texture_axis;
using assets::terrain::water_settings;

struct terrain {
   constexpr static std::size_t foliage_length =
      assets::terrain::terrain::foliage_length;
   constexpr static std::size_t texture_count = assets::terrain::terrain::texture_count;
   constexpr static std::size_t water_patch_size =
      assets::terrain::terrain::water_patch_size;
   constexpr static std::size_t foliage_patch_size =
      assets::terrain::terrain::foliage_patch_size;

   int32 length = 0;

   float height_scale = 1.0f;
   float grid_scale = 1.0f;

   active_flags active_flags;

   water_settings water_settings;

   std::array<std::string, texture_count> texture_names;
   std::array<float, texture_count> texture_scales{};
   std::array<texture_axis, texture_count> texture_axes{};
   std::string detail_texture_name;

   container::dynamic_array_2d<int16> heightmap{length, length};
   container::dynamic_array_2d<vec4> colormap_foreground{length, length};
   container::dynamic_array_2d<vec4> colormap_background{length, length};
   std::optional<container::dynamic_array_2d<vec4>> lightmap = std::nullopt;
   container::dynamic_array_2d<std::array<uint8, texture_count>> texture_weightmap{length, length};

   container::dynamic_array_2d<bool> water_patches{length / water_patch_size,
                                                   length / water_patch_size};

   container::dynamic_array_2d<foliage_patch> foliage_patches{foliage_length,
                                                              foliage_length};

   // TODO: Terraincuts.

   bool operator==(const terrain&) const noexcept = default;
};

}