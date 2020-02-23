#pragma once

#include "container/dynamic_array_2d.hpp"
#include "types.hpp"

#include <array>
#include <optional>
#include <string>

namespace sk::assets::terrain {

enum class version : uint32 { swbf = 21, swbf2 = 22 };

enum class texture_axis : uint8 {
   xz,
   xy,
   yz,
   zx,
   yx,
   zy,
   negative_xz,
   negative_xy,
   negative_yz,
   negative_zx,
   negative_yx,
   negative_zy
};

struct active_flags {
   bool terrain = true;
   bool water = true;
   bool foliage = true;

   bool operator==(const active_flags&) const noexcept = default;
};

struct water_settings {
   float height = 0.0f;
   float u_velocity = 0.0f;
   float v_velocity = 0.0f;
   float u_repeat = 0.0f;
   float v_repeat = 0.0f;
   float4 color = {1.0f, 1.0f, 1.0f, 1.0f};
   std::string texture;

   bool operator==(const water_settings&) const noexcept = default;
};

struct foliage_patch {
   bool layer0 : 1;
   bool layer1 : 1;
   bool layer2 : 1;
   bool layer3 : 1;

   bool operator==(const foliage_patch&) const noexcept = default;
};

struct terrain {
   constexpr static std::size_t foliage_length = 512;
   constexpr static std::size_t texture_count = 16;
   constexpr static std::size_t water_patch_size = 4;
   constexpr static std::size_t foliage_patch_size = 2;

   version version = version::swbf2;
   int32 length = 0;
   int16 active_left_offset = 0;
   int16 active_right_offset = 0;
   int16 active_top_offset = 0;
   int16 active_bottom_offset = 0;

   float height_scale = 1.0f;
   float grid_scale = 1.0f;

   active_flags active_flags;

   water_settings water_settings;

   std::array<std::string, texture_count> texture_names;
   std::array<float, texture_count> texture_scales;
   std::array<texture_axis, texture_count> texture_axes;
   std::string detail_texture_name;

   container::dynamic_array_2d<int16> heightmap{length, length};
   container::dynamic_array_2d<float4> colormap_foreground{length, length};
   container::dynamic_array_2d<float4> colormap_background{length, length};
   std::optional<container::dynamic_array_2d<float4>> lightmap = std::nullopt;
   container::dynamic_array_2d<std::array<uint8, texture_count>> texture_weightmap{length, length};

   container::dynamic_array_2d<bool> water_patches{length / water_patch_size,
                                                   length / water_patch_size};

   container::dynamic_array_2d<foliage_patch> foliage_patches{foliage_length,
                                                              foliage_length};

   // TODO: Terraincuts.
};

}