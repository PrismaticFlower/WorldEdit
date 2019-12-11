#pragma once

#include <types.hpp>

#include <array>
#include <optional>
#include <string>

#include <boost/multi_array.hpp>

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
};

struct water_settings {
   float height = 0.0f;
   float u_velocity = 0.0f;
   float v_velocity = 0.0f;
   float u_repeat = 0.0f;
   float v_repeat = 0.0f;
   glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
   std::string texture;
};

struct foliage_patch {
   bool layer0 : 1;
   bool layer1 : 1;
   bool layer2 : 1;
   bool layer3 : 1;

   bool operator==(const foliage_patch& other) const noexcept = default;
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

   boost::multi_array<int16, 2> heightmap{boost::extents[length][length]};
   boost::multi_array<vec4, 2> colormap_foreground{boost::extents[length][length]};
   boost::multi_array<vec4, 2> colormap_background{boost::extents[length][length]};
   std::optional<boost::multi_array<vec4, 2>> lightmap = std::nullopt;
   boost::multi_array<std::array<uint8, texture_count>, 2> texture_weightmap{
      boost::extents[length][length]};

   boost::multi_array<bool, 2> water_patches{
      boost::extents[length / water_patch_size][length / water_patch_size]};

   boost::multi_array<foliage_patch, 2> foliage_patches{
      boost::extents[foliage_length][foliage_length]};

   // TODO: Terraincuts.
};

}