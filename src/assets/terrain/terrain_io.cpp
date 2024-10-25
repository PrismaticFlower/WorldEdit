#include "terrain_io.hpp"
#include "io/output_file.hpp"
#include "utility/binary_reader.hpp"
#include "utility/enum_bitflags.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

using namespace std::literals;

namespace we::assets::terrain {

namespace {

constexpr uint32 terrain_max_string_length = 32;
constexpr int cluster_size = 4;
constexpr int foliage_patch_size = 2;
constexpr uint32 foliage_map_length = 512;
constexpr uint32 cluster_info_water_bit = 0x10000;

using terr_string = std::array<char, terrain_max_string_length>;

using terrain_string = std::array<char, terrain_max_string_length>;
;

static_assert(sizeof(terrain_string) == 32);

auto make_texture_name(const terrain_string& terrain_string) noexcept -> std::string
{
   std::string_view str{terrain_string};

   str = str.substr(0, str.find_first_of('\0'));

   if (string::iends_with(str, ".tga")) {
      str = str.substr(0, str.find_last_of('.'));
   }

   return std::string{str};
}

constexpr auto make_terrain_texture_string(const std::string_view str) noexcept -> terrain_string
{
   terrain_string out{};

   if (str.empty()) return out;

   constexpr auto append_suffix = ".tga\0"sv;

   const std::size_t out_size =
      std::min(str.size(), out.size() - append_suffix.size());

   std::copy_n(str.begin(), out_size, out.begin());
   std::copy_n(append_suffix.data(), append_suffix.size(), &out[out_size]);

   return out;
}

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
   int32 prelit;
   int32 terrain_length;
   int32 foliage_patch_size;
};

static_assert(sizeof(terrain_header) == 184);

enum class active_bitflags : uint8 {
   terrain = 0b1,
   water = 0b10,
   foliage = 0b100,
   has_extra_light_map = 0b1000
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

struct clusters_info {
   container::dynamic_array_2d<int16> min_heights;
   container::dynamic_array_2d<int16> max_heights;
   container::dynamic_array_2d<uint32> flags;
};

auto build_clusters_info(const terrain& terrain) -> clusters_info
{
   const auto clusters_length = terrain.length / cluster_size;

   clusters_info info{.min_heights{clusters_length, clusters_length},
                      .max_heights{clusters_length, clusters_length},
                      .flags{clusters_length, clusters_length}};

   for (int y = 0; y < terrain.length; y += cluster_size) {
      for (int x = 0; x < terrain.length; x += cluster_size) {
         int16 min_height = std::numeric_limits<int16>::min();
         int16 max_height = std::numeric_limits<int16>::max();

         for (int local_y = 0; local_y < cluster_size; ++local_y) {
            for (int local_x = 0; local_x < cluster_size; ++local_x) {
               const int16 height = terrain.height_map[{x + local_x, y + local_y}];

               min_height = std::min(height, min_height);
               max_height = std::max(height, max_height);
            }
         }

         info.min_heights[{x / cluster_size, (terrain.length - 1 - y) / cluster_size}] =
            min_height;
         info.max_heights[{x / cluster_size, (terrain.length - 1 - y) / cluster_size}] =
            max_height;
      }
   }

   for (int y = 0; y < terrain.length; y += cluster_size) {
      for (int x = 0; x < terrain.length; x += cluster_size) {
         uint32 flags = 0;

         // build texture vis mask
         for (int local_y = -1; local_y <= cluster_size; ++local_y) {
            for (int local_x = -1; local_x <= cluster_size; ++local_x) {
               for (uint32 i = 0; i < terrain::texture_count; ++i) {
                  const int abs_x = std::clamp(x + local_x, 0, terrain.length - 1);
                  const int abs_y = std::clamp(y + local_y, 0, terrain.length - 1);

                  const uint8 weight = terrain.texture_weight_maps[i][{abs_x, abs_y}];

                  flags |= (1 & (weight > 0)) << i;
               }
            }
         }

         if (terrain.water_map[{x / cluster_size, y / cluster_size}]) {
            flags |= cluster_info_water_bit;
         }

         info.flags[{x / cluster_size, (terrain.length - 1 - y) / cluster_size}] = flags;
      }
   }

   return info;
}

}

auto read_terrain(const std::span<const std::byte> bytes) -> terrain
{
   utility::binary_reader reader{bytes};

   const auto header = reader.read<terrain_header>();

   if (header.mn != std::array{'T', 'E', 'R', 'R'}) {
      throw std::runtime_error{
         ".ter file does not begin with 'TERR' and is likely corrupted!"};
   }

   if (header.terrain_length > 1024) {
      throw std::runtime_error{
         ".ter file length is too long. Terrain max length is 1024!"};
   }

   if (header.exheader_size != 164) {
      throw std::runtime_error{"Size of terrain exheader is unexpected."};
   }

   if (header.foliage_patch_size != foliage_patch_size) {
      throw std::runtime_error{
         ".ter file foliage patch size is not '2'! This is unsupported."};
   }

   terrain terrain{.version = header.version,
                   .length = header.active_right_offset - header.active_left_offset,
                   .height_scale = header.height_scale,
                   .grid_scale = header.grid_scale,
                   .prelit = header.prelit != 0,
                   .texture_scales = header.texture_scales,
                   .texture_axes = header.texture_axes};

   bool extra_light_map = false;

   // swbf2 active flags
   if (header.version == version::swbf2) {
      auto active = reader.read<active_bitflags>();

      terrain.active_flags.terrain =
         (active & active_bitflags::terrain) == active_bitflags::terrain;
      terrain.active_flags.water =
         (active & active_bitflags::water) == active_bitflags::water;
      terrain.active_flags.foliage =
         (active & active_bitflags::foliage) == active_bitflags::foliage;
      extra_light_map = (active & active_bitflags::has_extra_light_map) ==
                        active_bitflags::has_extra_light_map;
   }

   // texture names
   for (int i = 0; i < terrain.texture_count; ++i) {
      terrain.texture_names[i] = make_texture_name(reader.read<terrain_string>());

      auto detail_name = reader.read<terrain_string>();

      if (i == 0) {
         terrain.detail_texture_name = make_texture_name(detail_name);
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
   terrain.water_settings.texture = make_texture_name(water_settings[1].texture);

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
      if (extra_light_map) {
         terrain.light_map_extra =
            container::dynamic_array_2d<uint32>{terrain.length, terrain.length};

         read_map(terrain.light_map_extra);
      }
      read_map(texture_weight_map);

      // deinterleave texture weights
      for (int y = 0; y < terrain.length; ++y) {
         for (int x = 0; x < terrain.length; ++x) {
            for (int i = 0; i < terrain.texture_count; ++i) {
               terrain.texture_weight_maps[i][{x, y}] =
                  texture_weight_map[{x, y}][i];
            }
         }
      }

      const int loaded_cluster_length = terrain.length / cluster_size;
      const int cluster_length = header.terrain_length / cluster_size;
      const int cluster_count = cluster_length * cluster_length;
      const int cluster_active_offset = active_offset / cluster_size;
      const int cluster_active_end = active_end / cluster_size;

      reader.skip(cluster_count * sizeof(int16));
      reader.skip(cluster_count * sizeof(int16));

      for (int y = cluster_length - 1; y >= 0; --y) {
         reader.skip(cluster_active_offset * sizeof(uint32));

         if ((y >= cluster_active_offset) and (y < cluster_active_end)) {
            for (int x = 0; x < loaded_cluster_length; ++x) {
               uint32 cluster_flags = reader.read<uint32>();

               terrain.water_map[{x, y - cluster_active_offset}] =
                  (cluster_flags & cluster_info_water_bit) != 0;
            }
         }
         else {
            reader.skip(loaded_cluster_length * sizeof(uint32));
         }

         reader.skip(cluster_active_offset * sizeof(uint32));
      }

      container::dynamic_array_2d<uint8> foliage_map{foliage_map_length,
                                                     foliage_map_length};

      for (int y = foliage_map_length - 1; y >= 0; --y) {
         for (int x = 0; x < foliage_map_length; x += 2) {
            const uint8 packed_foliage = reader.read<uint8>();

            foliage_map[{x, y}] = (packed_foliage >> 4u) & 0xfu;
            foliage_map[{x + 1, y}] = packed_foliage & 0xfu;
         }
      }

      const int active_foliage_length = (terrain.length / 2);
      const int active_foliage_offset =
         (foliage_map_length - active_foliage_length) / 2;

      for (int y = 0; y < active_foliage_length; ++y) {
         for (int x = 0; x < active_foliage_length; ++x) {
            const uint8 foliage =
               foliage_map[{x + active_foliage_offset, y + active_foliage_offset}];

            terrain.foliage_map[{x, y}] = {.layer0 = (foliage & 0b1) != 0,
                                           .layer1 = (foliage & 0b10) != 0,
                                           .layer2 = (foliage & 0b100) != 0,
                                           .layer3 = (foliage & 0b1000) != 0};
         }
      }

      const std::size_t unused_sections_size = 262'144 + 131'072;

      reader.skip(unused_sections_size);

#if 0
      // This is how you would read terrain cuts in. We have to recreate them based off
      // world objects so we don't do this but it is kept around for completeness sake.

      [[maybe_unused]] const uint32 terrain_cuts_size = reader.read<uint32>();
      const uint32 terrain_cut_count = reader.read<uint32>();

      terrain.cuts.reserve(terrain_cut_count);

      for (uint32 i = 0; i < terrain_cut_count; ++i) {
         const uint32 plane_count = reader.read<uint32>();

         const float3 bbox_min = reader.read<float3>();
         const float3 bbox_max = reader.read<float3>();

         const float3 flipped_bbox_min = {bbox_min.x, bbox_min.y, -bbox_max.z};
         const float3 flipped_bbox_max = {bbox_max.x, bbox_max.y, -bbox_max.z};

         std::vector<float4> planes;
         planes.reserve(plane_count);

         for (uint32 plane_index = 0; plane_index < plane_count; ++plane_index) {
            float4 plane = reader.read<float4>();

            plane.z = -plane.z;

            planes.push_back(plane);
         }

         terrain.cuts.emplace_back(math::bounding_box{flipped_bbox_min, flipped_bbox_max},
                                   std::move(planes));
      }
#endif
   }
   catch (utility::binary_reader_overflow&) {
      // some .ter files in the stock assets end without all their data present
      // to ensure they load we catch the exception and just let the data be the default (0)
   }

   terrain.untracked_fill_dirty_rects();

   return terrain;
}

void save_terrain(const io::path& path, const terrain& terrain,
                  const std::span<const terrain_cut> terrain_cuts)
{
   io::output_file file{path};

   const int16 half_length = static_cast<int16>(terrain.length / 2);

   terrain_header header{.mn = {'T', 'E', 'R', 'R'},
                         .version = terrain.version,

                         .active_left_offset = -half_length,
                         .active_top_offset = -half_length,
                         .active_right_offset = half_length,
                         .active_bottom_offset = half_length,

                         .exheader_size = 164,

                         .texture_scales = terrain.texture_scales,
                         .texture_axes = terrain.texture_axes,

                         .height_scale = terrain.height_scale,
                         .grid_scale = terrain.grid_scale,
                         .prelit = terrain.prelit,
                         .terrain_length = terrain.length,
                         .foliage_patch_size = 2};

   file.write_object(header);

   if (terrain.version == version::swbf2) {
      active_bitflags flags{};

      // clang-format off
      if (terrain.active_flags.terrain)        flags |= active_bitflags::terrain;
      if (terrain.active_flags.foliage)        flags |= active_bitflags::foliage;
      if (terrain.active_flags.water)          flags |= active_bitflags::water;
      if (not terrain.light_map_extra.empty()) flags |= active_bitflags::has_extra_light_map;
      // clang-format on

      file.write_object(flags);
   }

   for (std::size_t i = 0; i < terrain.texture_count; ++i) {
      file.write_object(make_terrain_texture_string(terrain.texture_names[i]));

      if (i == 0) {
         file.write_object(make_terrain_texture_string(terrain.detail_texture_name));
      }
      else {
         file.write_object(terrain_string{}); // write empty strings for all other detail textures
      }
   }

   terrain_water_settings water_settings{
      .height = terrain.water_settings.height,
      .u_velocity = terrain.water_settings.u_velocity,
      .v_velocity = terrain.water_settings.v_velocity,
      .u_repeat = terrain.water_settings.u_repeat,
      .v_repeat = terrain.water_settings.v_repeat,
      .colour = utility::pack_srgb_bgra(terrain.water_settings.color),
      .texture = make_terrain_texture_string(terrain.water_settings.texture)};

   file.write_object(terrain_water_settings{}); // write null unused water settings
   file.write_object(water_settings);           // write actual water settings
   for (int i = 0; i < 14; ++i) file.write_object(terrain_water_settings{});

   // unused decal stuff

   // decal names
   for (int i = 0; i < 16; ++i) file.write_object(terrain_string{});

   // decal tile counts
   file.write_object(int32{0});

   // unused foliage stuff
   file.write_object(int32{0});
   file.write_object(int32{0});

   auto write_map = [&](const auto& map) {
      for (int y = int{terrain.length} - 1; y >= 0; --y) {
         file.write(std::as_bytes(std::span{&map[{0, y}], map.width()}));
      }
   };

   write_map(terrain.height_map);
   write_map(terrain.color_map);
   write_map(terrain.light_map);
   if (terrain.version == version::swbf2 and not terrain.light_map_extra.empty()) {
      write_map(terrain.light_map_extra);
   }

   // interleave texture weights
   for (int y = int{terrain.length} - 1; y >= 0; --y) {
      for (int x = 0; x < int{terrain.length}; ++x) {
         std::array<uint8, terrain::texture_count> weights;

         for (std::size_t slice = 0; slice < terrain::texture_count; ++slice) {
            weights[slice] = terrain.texture_weight_maps[slice][{x, y}];
         }

         file.write_object(weights);
      }
   }

   const clusters_info info = build_clusters_info(terrain);

   file.write(std::as_bytes(std::span{info.min_heights}));
   file.write(std::as_bytes(std::span{info.max_heights}));
   file.write(std::as_bytes(std::span{info.flags}));

   // foliage

   const int active_foliage_length = (terrain.length / 2);
   const int active_foliage_offset = (foliage_map_length - active_foliage_length) / 2;

   for (int y = 0; y < active_foliage_offset; ++y) {
      file.write(std::array<std::byte, 256>{});
   }

   for (int y = 0; y < active_foliage_length; ++y) {
      for (int x = 0; x < (active_foliage_offset / 2); ++x) {
         file.write_object(std::byte{});
      }

      const int flipped_y = (active_foliage_length - 1) - y;

      for (int x = 0; x < active_foliage_length; x += 2) {
         const foliage_patch foliage_0 = terrain.foliage_map[{x, flipped_y}];
         const foliage_patch foliage_1 = terrain.foliage_map[{x + 1, flipped_y}];

         uint8 packed_foliage = 0;

         packed_foliage |= foliage_1.layer0 << 0;
         packed_foliage |= foliage_1.layer1 << 1;
         packed_foliage |= foliage_1.layer2 << 2;
         packed_foliage |= foliage_1.layer3 << 3;
         packed_foliage |= foliage_0.layer0 << 4;
         packed_foliage |= foliage_0.layer1 << 5;
         packed_foliage |= foliage_0.layer2 << 6;
         packed_foliage |= foliage_0.layer3 << 7;

         file.write_object(packed_foliage);
      }

      for (int x = 0; x < (active_foliage_offset / 2); ++x) {
         file.write_object(std::byte{});
      }
   }

   for (int y = 0; y < active_foliage_offset; ++y) {
      file.write(std::array<std::byte, 256>{});
   }

   const std::size_t unused_sections_size = 262'144 + 131'072;

   const std::vector<std::byte> dummy_data{unused_sections_size};
   const std::span<const std::byte> dummy_data_span{dummy_data};

   file.write(dummy_data_span);

   std::size_t terrain_cuts_size = sizeof(uint32);

   for (const auto& cut : terrain_cuts) {
      terrain_cuts_size +=
         sizeof(uint32) + sizeof(float3) * 2 + sizeof(float4) * cut.planes.size();
   }

   file.write_object(static_cast<uint32>(terrain_cuts_size));
   file.write_object(static_cast<uint32>(terrain_cuts.size()));

   for (const auto& cut : terrain_cuts) {
      file.write_object(static_cast<uint32>(cut.planes.size()));

      const float3 bbox_min = {cut.bbox.min.x, cut.bbox.min.y, -cut.bbox.max.z};
      const float3 bbox_max = {cut.bbox.max.x, cut.bbox.max.y, -cut.bbox.min.z};

      file.write_object(bbox_min);
      file.write_object(bbox_max);

      for (float4 plane : cut.planes) {
         plane.z = -plane.z;

         file.write_object(plane);
      }
   }
}
}
