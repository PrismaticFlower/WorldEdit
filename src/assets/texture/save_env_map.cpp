#include "save_env_map.hpp"
#include "io/memory_mapped_file.hpp"

#include <array>
#include <filesystem>

namespace we::assets::texture {

namespace {
constexpr uint8 rgba_image_type = 2;

struct tga_header {
   uint8 image_id_length = 0;
   uint8 color_map_type = 0;
   uint8 image_type = rgba_image_type;
   uint8 color_map_index[2] = {0, 0};
   uint8 color_map_length[2] = {0, 0};
   uint8 color_map_entry_size = 0;
   uint16 image_x_origin = 0;
   uint16 image_y_origin = 0;
   uint16 image_width;
   uint16 image_height;
   uint8 image_pixel_depth = 24;
   uint8 image_description = 0;
};

using dest_texel = std::array<std::byte, 3>;
static_assert(sizeof(dest_texel) == 3);

void clear_region(dest_texel* dest_data, uint32 length, uint32 row_pitch)
{
   for (uint32 y = 0; y < length; ++y) {
      std::memset(&dest_data[y * row_pitch], 0, sizeof(dest_texel) * length);
   }
}

}

void save_env_map(const std::filesystem::path& path, const env_map_view env_map)
{
   io::memory_mapped_file file{
      io::memory_mapped_file_params{.path = path.c_str(),
                                    .size = sizeof(tga_header) +
                                            (env_map.length * env_map.length *
                                             sizeof(dest_texel) * 4 * 3),
                                    .truncate_to_size = true}};

   const uint32 image_width = env_map.length * 4;
   const uint32 image_height = env_map.length * 3;

   new (file.data()) tga_header{.image_width = static_cast<uint16>(image_width),
                                .image_height = static_cast<uint16>(image_height)};

   dest_texel* const image_data =
      reinterpret_cast<dest_texel*>(file.data() + sizeof(tga_header));

   const std::array<std::array<uint32, 2>, 6> item_offsets = {{
      {2 * env_map.length, 1 * env_map.length}, // +X
      {0 * env_map.length, 1 * env_map.length}, // -X
      {1 * env_map.length, 2 * env_map.length}, // +Y
      {1 * env_map.length, 0 * env_map.length}, // -Y
      {1 * env_map.length, 1 * env_map.length}, // +Z
      {3 * env_map.length, 1 * env_map.length}  // -Z
   }};

   for (uint32 i = 0; i < 6; ++i) {
      const std::array<uint32, 2> offset = item_offsets[i];

      const std::byte* const face_data =
         env_map.data.subspan(i * env_map.item_pitch, env_map.item_pitch).data();

      const uint32 length_minus_1 = env_map.length - 1;

      for (uint32 y = 0; y < env_map.length; ++y) {
         dest_texel* dest_data =
            image_data + ((offset[1] + (length_minus_1 - y)) * image_width) +
            offset[0];
         const std::byte* src_data = face_data + y * env_map.row_pitch;

         for (uint32 x = 0; x < env_map.length; ++x) {
            new (dest_data) dest_texel{src_data[0], src_data[1], src_data[2]};

            dest_data += 1;
            src_data += 4;
         }
      }
   }

   const std::array<std::array<uint32, 2>, 6> clear_offsets = {{
      {0 * env_map.length, 0 * env_map.length},
      {2 * env_map.length, 0 * env_map.length}, // -X
      {3 * env_map.length, 0 * env_map.length}, // +Y
      {0 * env_map.length, 2 * env_map.length}, // -Y
      {2 * env_map.length, 2 * env_map.length}, // +Z
      {3 * env_map.length, 2 * env_map.length}  // -Z
   }};

   for (const auto& offset : clear_offsets) {
      clear_region(image_data + (offset[1] * image_width) + offset[0],
                   env_map.length, image_width);
   }

   file.reset();

   std::filesystem::last_write_time(path, std::chrono::file_clock::now());
}

}
