#pragma once

#include "container/dynamic_array_2d.hpp"
#include "dirty_rect_tracker.hpp"
#include "math/bounding_box.hpp"
#include "types.hpp"

#include <array>
#include <string>
#include <vector>

namespace we::assets::terrain {

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

struct terrain_cut {
   math::bounding_box bbox;
   std::vector<float4> planes;
};

struct terrain {
   constexpr static std::size_t texture_count = 16;

   version version = version::swbf2;
   int32 length = 64;

   float height_scale = 1.0f;
   float grid_scale = 1.0f;

   active_flags active_flags;

   bool prelit = false;

   water_settings water_settings;

   std::array<std::string, texture_count> texture_names;
   std::array<float, texture_count> texture_scales;
   std::array<texture_axis, texture_count> texture_axes;
   std::string detail_texture_name;

   container::dynamic_array_2d<int16> height_map{length, length};
   container::dynamic_array_2d<uint32> color_map{length, length};
   container::dynamic_array_2d<uint32> light_map{length, length};
   container::dynamic_array_2d<uint32> light_map_extra;
   std::array<container::dynamic_array_2d<uint8>, texture_count> texture_weight_maps =
      {container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length},
       container::dynamic_array_2d<uint8>{length, length}};

   container::dynamic_array_2d<bool> water_map{length / 4, length / 4};

   container::dynamic_array_2d<foliage_patch> foliage_map{length / 2, length / 2};

   dirty_rect_tracker height_map_dirty;
   std::array<dirty_rect_tracker, texture_count> texture_weight_maps_dirty;
   dirty_rect_tracker color_or_light_map_dirty;
   dirty_rect_tracker water_map_dirty;
   dirty_rect_tracker foliage_map_dirty;

   void untracked_fill_dirty_rects() noexcept;

   void untracked_clear_dirty_rects() noexcept;
};

}
