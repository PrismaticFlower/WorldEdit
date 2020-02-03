#include "terrain_io.hpp"
#include "utility/binary_reader.hpp"
#include "utility/enum_bitflags.hpp"
#include "utility/srgb_conversion.hpp"

#include <algorithm>
#include <stdexcept>

namespace sk::assets::terrain {

namespace {

constexpr int terr_max_string_length = 32;

using terr_string = std::array<char, terr_max_string_length>;

struct terr_header {
   std::array<char, 4> mn;
   version version;
   int16 active_left_offset;
   int16 active_top_offset;
   int16 active_right_offset;
   int16 active_bottom_offset;
   int32 unknown;
   std::array<float, 16> tex_scales;
   std::array<texture_axis, 16> tex_axes;
   std::array<float, 16> tex_rotations;
   float height_scale;
   float grid_scale;
   int32 prelit;
   int32 terrain_length;
   int32 foliage_patch_size;
};

static_assert(sizeof(terr_header) == 184);

enum class active_bitflags : uint8 {
   terrain = 0b1,
   water = 0b10,
   foliage = 0b100
};

constexpr bool marked_as_enum_bitflag(active_bitflags)
{
   return true;
}

struct terr_water_settings {
   float height;
   std::array<float, 3> unused{};
   float u_velocity;
   float v_velocity;
   float u_repeat;
   float v_repeat;
   uint32 colour;
   terr_string texture;
};

static_assert(sizeof(terr_water_settings) == 68);

struct terr_decal_tile {
   int32 x;
   int32 y;
   int32 texture_index;

   std::array<vec2, 4> coords;
};

static_assert(sizeof(terr_decal_tile) == 44);

enum class terr_patch_flags : uint16 {
   visible = 0x1,
};

constexpr bool marked_as_enum_bitflag(terr_patch_flags)
{
   return true;
}

struct terr_patch_info {
   terr_patch_flags flags;
   uint8 water_layer;
   uint8 unknown;
};

static_assert(sizeof(terr_patch_info) == 4);

auto make_string(terr_string str) -> std::string
{
   return {str.cbegin(), std::find(str.cbegin(), str.cend(), '\0')};
}

}

auto read_terrain(const gsl::span<const std::byte> bytes) -> terrain
{
   utility::binary_reader reader{bytes};

   auto header = reader.read<terr_header>();

   if (header.mn != std::array{'T', 'E', 'R', 'R'}) {
      throw std::runtime_error{
         ".ter file does not begin with 'TERR' and is likely corrupted!"};
   }

   if (header.version != version::swbf2) {
      throw std::runtime_error{".ter file version is not '22'! Support for "
                               "SWBF1 .ter files has not been implemented."};
   }

   if (header.foliage_patch_size != 2) {
      throw std::runtime_error{
         ".ter file foliage patche size is not '2'! This is unsupported."};
   }

   terrain terrain{.version = header.version,
                   .length = header.terrain_length,
                   .active_left_offset = header.active_left_offset,
                   .active_right_offset = header.active_right_offset,
                   .active_top_offset = header.active_top_offset,
                   .active_bottom_offset = header.active_bottom_offset,
                   .height_scale = header.height_scale,
                   .grid_scale = header.grid_scale,
                   .texture_scales = header.tex_scales,
                   .texture_axes = header.tex_axes};

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
      auto name = reader.read<terr_string>();

      terrain.texture_names[i] = make_string(name);

      auto detail_name = reader.read<terr_string>();

      if (i == 0) {
         terrain.detail_texture_name = make_string(detail_name);
      }
   }

   // water settings
   auto water_settings = reader.read<std::array<terr_water_settings, 16>>();

   // only water_settings[1] is used by the game engine

   terrain.water_settings.height = water_settings[1].height;
   terrain.water_settings.u_repeat = water_settings[1].u_repeat;
   terrain.water_settings.v_repeat = water_settings[1].v_repeat;
   terrain.water_settings.u_velocity = water_settings[1].u_velocity;
   terrain.water_settings.v_velocity = water_settings[1].v_velocity;
   terrain.water_settings.color = utility::unpack_srgb_bgra(water_settings[1].colour);
   terrain.water_settings.texture = make_string(water_settings[1].texture);

   // decals
   reader.read<std::array<terr_string, 16>>(); // (unused) decal textures
   auto decal_tile_count = reader.read<int32>();

   if (decal_tile_count > 0) {
      for (int i = 0; i < decal_tile_count; ++i) {
         reader.read<terr_decal_tile>();
      }
   }
   else {
      reader.read<std::array<uint32, 2>>(); // decal padding
   }

   // height map
   for (int y = 0; y < terrain.length; ++y) {
      for (int x = 0; x < terrain.length; ++x) {
         terrain.heightmap[{x, y}] = reader.read<uint16>();
      }
   }

   // color map foreground
   for (int y = 0; y < terrain.length; ++y) {
      for (int x = 0; x < terrain.length; ++x) {
         terrain.colormap_foreground[{x, y}] =
            utility::unpack_srgb_bgra(reader.read<uint32>());
      }
   }

   // color map background
   for (int y = 0; y < terrain.length; ++y) {
      for (int x = 0; x < terrain.length; ++x) {
         terrain.colormap_background[{x, y}] =
            utility::unpack_srgb_bgra(reader.read<uint32>());
      }
   }

   // light map
   if (header.prelit) {
      terrain.lightmap.emplace(terrain.length, terrain.length);

      for (int y = 0; y < terrain.length; ++y) {
         for (int x = 0; x < terrain.length; ++x) {
            (*terrain.lightmap)[{x, y}] =
               utility::unpack_srgb_bgra(reader.read<uint32>());
         }
      }
   }

   // texture weight map
   for (int y = 0; y < terrain.length; ++y) {
      for (int x = 0; x < terrain.length; ++x) {
         terrain.texture_weightmap[{x, y}] =
            reader.read<std::array<uint8, terrain::texture_count>>();
      }
   }

   // skip unknown data #1
   reader.skip((terrain.length / 2) * (terrain.length / 4));

   // skip unknown data #2
   reader.skip((terrain.length / 2) * (terrain.length / 4));

   // terrain patches
   {
      for (int y = 0; y < (terrain.length / terrain.water_patch_size); ++y) {
         for (int x = 0; x < (terrain.length / terrain.water_patch_size); ++x) {
            terrain.water_patches[{x, y}] =
               (reader.read<terr_patch_info>().water_layer == 1);
         }
      }
   }

   // foliage map
   {
      for (int y = 0; y < terrain.foliage_length; ++y) {
         for (int x = 0; x < (terrain.foliage_length / 2); ++x) {
            auto foliage = reader.read<uint8>();

            terrain.foliage_patches[{x * 2, y}] = {.layer0 = (foliage & 0b1) != 0,
                                                   .layer1 = (foliage & 0b10) != 0,
                                                   .layer2 = (foliage & 0b100) != 0,
                                                   .layer3 = (foliage & 0b1000) != 0};

            terrain.foliage_patches[{x * 2 + 1, y}] =
               {.layer0 = (foliage & 0b10000) != 0,
                .layer1 = (foliage & 0b100000) != 0,
                .layer2 = (foliage & 0b1000000) != 0,
                .layer3 = (foliage & 0b10000000) != 0};
         }
      }
   }

   // TODO: Terrain cuts. (Although there isn't much point in reading them as we'll just have to recreate them based off objects in the world.)

   return terrain;
}

}
