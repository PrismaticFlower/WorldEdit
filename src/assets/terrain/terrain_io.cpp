#include "terrain_io.hpp"
#include "utility/binary_reader.hpp"
#include "utility/enum_bitflags.hpp"
#include "utility/srgb_conversion.hpp"

#include <algorithm>
#include <stdexcept>

namespace sk::assets::terrain {

namespace {

constexpr uint32 terrain_max_string_length = 32;

using terr_string = std::array<char, terrain_max_string_length>;

struct terrain_string {
   std::array<char, terrain_max_string_length> string;

   operator std::string() const noexcept
   {
      return {string.cbegin(), std::find(string.cbegin(), string.cend(), '\0')};
   }
};

static_assert(sizeof(terrain_string) == 32);

struct terrain_header {
   std::array<char, 4> mn;
   version version;
   int16 active_left_offset;
   int16 active_top_offset;
   int16 active_right_offset;
   int16 active_bottom_offset;
   uint32 exheader_size;
   std::array<float, 16> texture_scales;
   std::array<texture_axis, 16> texture_axes;
   std::array<float, 16> texture_rotations;
   float height_scale;
   float grid_scale;
   int32 extra_light_map;
   int32 terrain_length;
   int32 foliage_patch_size;
};

static_assert(sizeof(terrain_header) == 184);

enum class active_bitflags : uint8 {
   terrain = 0b1,
   water = 0b10,
   foliage = 0b100
};

constexpr bool marked_as_enum_bitflag(active_bitflags)
{
   return true;
}

struct terrain_water_settings {
   float height;
   std::array<float, 3> unused{};
   float u_velocity;
   float v_velocity;
   float u_repeat;
   float v_repeat;
   uint32 colour;
   terrain_string texture;
};

static_assert(sizeof(terrain_water_settings) == 68);

struct terrain_decal_tile {
   int32 x;
   int32 y;
   int32 texture_index;

   std::array<float2, 4> coords;
};

static_assert(sizeof(terrain_decal_tile) == 44);

}

auto read_terrain(const std::span<const std::byte> bytes) -> terrain
{
   utility::binary_reader reader{bytes};

   const auto header = reader.read<terrain_header>();

   if (header.mn != std::array{'T', 'E', 'R', 'R'}) {
      throw std::runtime_error{
         ".ter file does not begin with 'TERR' and is likely corrupted!"};
   }

   if (header.exheader_size != 164) {
      throw std::runtime_error{"Size of terrain exheader is unexpected."};
   }

   if (header.foliage_patch_size != 2) {
      throw std::runtime_error{
         ".ter file foliage patche size is not '2'! This is unsupported."};
   }

   terrain terrain{.version = header.version,
                   .length = header.active_right_offset - header.active_left_offset,
                   .height_scale = header.height_scale,
                   .grid_scale = header.grid_scale,
                   .texture_scales = header.texture_scales,
                   .texture_axes = header.texture_axes};

   // swbf2 active flags
   if (header.version == version::swbf2) {
      auto active = reader.read<active_bitflags>();

      terrain.active_flags.terrain =
         (active & active_bitflags::terrain) == active_bitflags::terrain;
      terrain.active_flags.water =
         (active & active_bitflags::water) == active_bitflags::water;
      terrain.active_flags.foliage =
         (active & active_bitflags::foliage) == active_bitflags::foliage;
   }

   // texture names
   for (int i = 0; i < terrain.texture_count; ++i) {
      terrain.texture_names[i] = reader.read<terrain_string>();

      auto detail_name = reader.read<terrain_string>();

      if (i == 0) {
         terrain.detail_texture_name = detail_name;
      }
   }

   // water settings
   const auto water_settings = reader.read<std::array<terrain_water_settings, 16>>();

   // only water_settings[1] is used by the game engine

   terrain.water_settings.height = water_settings[1].height;
   terrain.water_settings.u_repeat = water_settings[1].u_repeat;
   terrain.water_settings.v_repeat = water_settings[1].v_repeat;
   terrain.water_settings.u_velocity = water_settings[1].u_velocity;
   terrain.water_settings.v_velocity = water_settings[1].v_velocity;
   terrain.water_settings.color = utility::unpack_srgb_bgra(water_settings[1].colour);
   terrain.water_settings.texture = water_settings[1].texture;

   // decals
   reader.read<std::array<terrain_string, 16>>(); // (unused) decal textures
   const auto decal_tile_count = reader.read<int32>();

   for (int i = 0; i < decal_tile_count; ++i) {
      reader.read<terrain_decal_tile>();
   }

   // unused foliage stuff

   const auto foliage_cluster_count = reader.read<int32>();

   for (int i = 0; i < foliage_cluster_count; ++i) {
      reader.read<std::array<uint32, 6>>();
   }

   const auto foliage_model_count = reader.read<int32>();

   for (int i = 0; i < foliage_model_count; ++i) {
      reader.read<std::array<uint32, 7>>();
   }

   const int active_offset = (header.terrain_length - terrain.length) / 2;
   const int active_end = active_offset + terrain.length;

   const auto read_map = [&]<typename T>(container::dynamic_array_2d<T>& out) {
      for (int y = header.terrain_length - 1; y >= 0; --y) {
         reader.skip(active_offset * sizeof(T));

         if ((y >= active_offset) and (y < active_end)) {
            for (int x = 0; x < terrain.length; ++x) {
               out[{x, y - active_offset}] = reader.read<T>();
            }
         }
         else {
            reader.skip(terrain.length * sizeof(T));
         }

         reader.skip(active_offset * sizeof(T));
      }
   };

   container::dynamic_array_2d<std::array<uint8, terrain::texture_count>>
      texture_weight_map{terrain.length, terrain.length};

   try {
      read_map(terrain.height_map);
      read_map(terrain.color_map);
      read_map(terrain.light_map);
      if (header.extra_light_map) read_map(terrain.light_map_extra);
      read_map(texture_weight_map);
   }
   catch (utility::binary_reader_overflow&) {
      // some .ter files in the stock assets end without all their data present
      // to ensure they load we catch the exception and just let the data be the default (0)
   }

   // deinterleave texture weights
   for (int y = 0; y < terrain.length; ++y) {
      for (int x = 0; x < terrain.length; ++x) {
         for (int i = 0; i < terrain.texture_count; ++i) {
            terrain.texture_weight_maps[i][{x, y}] = texture_weight_map[{x, y}][i];
         }
      }
   }

   // TODO: Water.

   // TODO: Foliage.

   // TODO: Terrain cuts. (Although there isn't much point in reading them as we'll just have to recreate them based off objects in the world.)

   return terrain;
}

}
