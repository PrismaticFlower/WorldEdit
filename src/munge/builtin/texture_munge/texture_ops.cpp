#include "texture_ops.hpp"
#include "error.hpp"

#include "math/vector_funcs.hpp"

#include <bit>
#include <vector>

#include <stb_image_resize2.h>

#include <fmt/format.h>

namespace we::munge {

namespace {

struct texture_heightmap {
   struct init_params {
      std::span<uint8> texels;
      uint32 width = 0;
      uint32 height = 0;
   };

   texture_heightmap() = default;

   texture_heightmap(const init_params& init_params)
      : _texels{init_params.texels},
        _width{init_params.width},
        _height{init_params.height}
   {
      assert(_width * _height <= _texels.size());
   }

   [[nodiscard]] auto width() const noexcept -> uint32
   {
      return _width;
   }

   [[nodiscard]] auto height() const noexcept -> uint32
   {
      return _height;
   }

   [[nodiscard]] auto at(std::size_t x, std::size_t y) const noexcept -> uint8
   {
      if (x >= _width) x = x % _width;
      if (y >= _height) y = y % _height;

      const std::size_t index = y * _width + x;

      assert(index < _texels.size());

      return _texels[index];
   }

   [[nodiscard]] auto at(std::size_t x, std::size_t y) noexcept -> uint8&
   {
      if (x >= _width) x = x % _width;
      if (y >= _height) y = y % _height;

      const std::size_t index = y * _width + x;

      assert(index < _texels.size());

      return _texels[index];
   }

private:
   std::span<uint8> _texels;
   uint32 _width = 0;
   uint32 _height = 0;
};

auto unpack_f3(const uint32 color) noexcept -> float3
{
   return {((color >> 16u) & 0xffu) / 255.0f, ((color >> 8u) & 0xffu) / 255.0f,
           (color & 0xffu) / 255.0f};
}

auto unpack_f4(const uint32 color) noexcept -> float4
{
   return {((color >> 16u) & 0xffu) / 255.0f, ((color >> 8u) & 0xffu) / 255.0f,
           (color & 0xffu) / 255.0f, ((color >> 24u) & 0xffu) / 255.0f};
}

auto pack(const float4& color) noexcept -> uint32
{
   return (static_cast<uint32>(color.x * 255.0f + 0.5f) << 16u) |
          (static_cast<uint32>(color.y * 255.0f + 0.5f) << 8u) |
          (static_cast<uint32>(color.z * 255.0f + 0.5f)) |
          (static_cast<uint32>(color.w * 255.0f + 0.5f) << 24u);
}

auto build_height_map(const texture_slice& texture,
                      std::vector<uint8>& height_map_storage) -> texture_heightmap
{
   if (height_map_storage.size() < texture.width() * texture.height()) {
      height_map_storage.resize(texture.width() * texture.height());
   }

   texture_heightmap heightmap =
      texture_heightmap::init_params{.texels = height_map_storage,
                                     .width = texture.width(),
                                     .height = texture.height()};

   for (uint32 y = 0; y < texture.height(); ++y) {
      for (uint32 x = 0; x < texture.width(); ++x) {
         heightmap.at(x, y) = static_cast<uint8>(texture.at(x, y) & 0xff);
      }
   }

   return heightmap;
}

void generate_normal_map(const texture_heightmap& input, texture_slice& output,
                         const float bump_scale)
{
   assert(input.width() == output.width());
   assert(input.height() == output.height());

   const float inv_height = 1.0f / 255.0f;
   const float scale = inv_height * bump_scale * 0.5f;

   for (uint32 y = 0; y < input.height(); ++y) {
      const uint32 y0 = y - 1;
      const uint32 y1 = y + 1;

      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 x0 = x - 1;
         const uint32 x1 = x + 1;

         const int32 height0x = input.at(x0, y);
         const int32 height1x = input.at(x1, y);
         const int32 height0y = input.at(x, y0);
         const int32 height1y = input.at(x, y1);

         const float3 normal = normalize(float3{(height0x - height1x) * scale,
                                                (height0y - height1y) * scale, 1.0f});

         uint32& value = output.at(x, y);

         value &= 0xff'00'00'00u;
         value |= static_cast<uint32>(normal.x * 127.5f + 128.0f) << 16u;
         value |= static_cast<uint32>(normal.y * 127.5f + 128.0f) << 8u;
         value |= static_cast<uint32>(normal.z * 127.5f + 128.0f) << 0u;
      }
   }
}

void generate_normal_map_highq(const texture_heightmap& input,
                               texture_slice& output, const float bump_scale)
{
   assert(input.width() == output.width());
   assert(input.height() == output.height());

   const float inv_height = 1.0f / 255.0f;
   const float scale = inv_height * bump_scale * 0.5f;

   for (uint32 y = 0; y < input.height(); ++y) {
      const uint32 y0 = y - 1;
      const uint32 y1 = y + 1;

      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 x0 = x - 1;
         const uint32 x1 = x + 1;

         const float height = input.at(x, y) * scale;
         const float height0x = input.at(x0, y) * scale;
         const float height1x = input.at(x1, y) * scale;
         const float height0y = input.at(x, y0) * scale;
         const float height1y = input.at(x, y1) * scale;

         const float3 v{0.0f, 0.0f, height};
         const float3 v_x0{-1.0f, 0.0f, height0x};
         const float3 v_x1{1.0f, 0.0f, height1x};
         const float3 v_y0{0.0f, -1.0f, height0y};
         const float3 v_y1{0.0f, 1.0f, height1y};

         const float3 e0 = v - v_x0;
         const float3 e1 = v - v_y0;
         const float3 e2 = v - v_x1;
         const float3 e3 = v - v_y1;

         const float3 normal =
            normalize(cross(e0, e1) + cross(e1, e2) + cross(e2, e3) + cross(e3, e0));

         uint32& value = output.at(x, y);

         value &= 0xff'00'00'00u;
         value |= static_cast<uint32>(normal.x * 127.5f + 128.0f) << 16u;
         value |= static_cast<uint32>(normal.y * 127.5f + 128.0f) << 8u;
         value |= static_cast<uint32>(normal.z * 127.5f + 128.0f) << 0u;
      }
   }
}

}

void convert_to_detail_map(texture& texture, bool has_alpha)
{
   bool all_black = true;
   bool all_white = true;

   if (has_alpha) {
      for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
         texture_subresource subresource =
            texture.subresource({.array_index = array_index});

         for (uint32 z = 0; z < subresource.depth(); ++z) {
            texture_slice slice = subresource.slice(z);

            for (uint32 y = 0; y < slice.height(); ++y) {
               for (uint32 x = 0; x < slice.width(); ++x) {
                  const uint32 color = slice.at(x, y) & 0xff'ff'ff;

                  all_black &= color == 0x00'00'00;
                  all_white &= color == 0xff'ff'ff;

                  if (not all_black and not all_white) break;
               }
            }
         }
      }
   }

   if (has_alpha and (all_black or all_white)) {
      for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
         texture_subresource subresource =
            texture.subresource({.array_index = array_index});

         for (uint32 z = 0; z < subresource.depth(); ++z) {
            texture_slice slice = subresource.slice(z);

            for (uint32 y = 0; y < slice.height(); ++y) {
               for (uint32 x = 0; x < slice.width(); ++x) {
                  uint32& color = slice.at(x, y);

                  const uint32 alpha = (color & 0xff'00'00'00) >> 24;

                  color = 0xff'00'00'00;
                  color |= alpha << 16u;
                  color |= alpha << 8u;
                  color |= alpha;
               }
            }
         }
      }
   }
   else {
      for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
         texture_subresource subresource =
            texture.subresource({.array_index = array_index});

         for (uint32 z = 0; z < subresource.depth(); ++z) {
            texture_slice slice = subresource.slice(z);

            for (uint32 y = 0; y < slice.height(); ++y) {
               for (uint32 x = 0; x < slice.width(); ++x) {
                  uint32& color = slice.at(x, y);

                  const float luma =
                     dot(unpack_f3(color), float3{0.299f, 0.587f, 0.114f});

                  const uint32 u8_luma = static_cast<uint32>(luma * 255.0f + 0.5f);

                  color = 0xff'00'00'00;
                  color |= u8_luma << 16u;
                  color |= u8_luma << 8u;
                  color |= u8_luma;
               }
            }
         }
      }
   }
}

void make_opaque(texture& texture) noexcept
{
   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      texture_subresource subresource =
         texture.subresource({.array_index = array_index, .mip_level = 0});

      for (uint32 z = 0; z < subresource.depth(); ++z) {
         texture_slice slice = subresource.slice(z);

         for (uint32 y = 0; y < slice.height(); ++y) {
            for (uint32 x = 0; x < slice.width(); ++x) {
               slice.at(x, y) |= 0xff'00'00'00;
            }
         }
      }
   }
}

void adjust_saturation(texture& texture, float saturation) noexcept
{
   saturation *= 2.0f;

   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      texture_subresource subresource =
         texture.subresource({.array_index = array_index});

      for (uint32 z = 0; z < subresource.depth(); ++z) {
         texture_slice slice = subresource.slice(z);

         for (uint32 y = 0; y < slice.height(); ++y) {
            for (uint32 x = 0; x < slice.width(); ++x) {
               uint32& color = slice.at(x, y);

               float3 unpacked = unpack_f3(color);

               const float luma = dot(unpacked, float3{0.299f, 0.587f, 0.114f});

               unpacked = saturate(lerp({luma, luma, luma}, unpacked, saturation));

               color &= 0xff'00'00'00;
               color |= static_cast<uint32>(unpacked.x * 255.0f + 0.5f) << 16u;
               color |= static_cast<uint32>(unpacked.y * 255.0f + 0.5f) << 8u;
               color |= static_cast<uint32>(unpacked.z * 255.0f + 0.5f);
            }
         }
      }
   }
}

void generate_mipmaps(texture& texture)
{
   if (texture.depth() <= 1) {
      for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
         for (uint32 mip = 1; mip < texture.mip_levels(); ++mip) {
            const texture_slice input =
               texture
                  .subresource({.array_index = array_index, .mip_level = mip - 1})
                  .slice(0);
            texture_slice output =
               texture
                  .subresource({.array_index = array_index, .mip_level = mip})
                  .slice(0);

            stbir_resize_uint8_linear(
               reinterpret_cast<const unsigned char*>(input.as_bytes().data()),
               input.width(), input.height(), input.width() * sizeof(uint32),
               reinterpret_cast<unsigned char*>(output.as_bytes().data()),
               output.width(), output.height(), output.width() * sizeof(uint32),
               STBIR_BGRA_NO_AW);
         }
      }
   }
   else {
      if (not std::has_single_bit(texture.width()) or
          not std::has_single_bit(texture.height()) or
          not std::has_single_bit(texture.depth())) {
         throw texture_error{
            fmt::format("Can not generate mipmaps for non-power of 2 "
                        "volume texture. Texture size: {}x{}x{}",
                        texture.width(), texture.height(), texture.depth()),
            texture_ec::generate_mipmaps_volume_non_pow2};
      }

      for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
         for (uint32 mip = 1; mip < texture.mip_levels(); ++mip) {
            const texture_subresource input = texture.subresource(
               {.array_index = array_index, .mip_level = mip - 1});
            texture_subresource output =
               texture.subresource({.array_index = array_index, .mip_level = mip});

            for (uint32 z = 0; z < output.depth(); ++z) {
               for (uint32 y = 0; y < output.height(); ++y) {
                  for (uint32 x = 0; x < output.width(); ++x) {
                     float4 color = {};

                     color += unpack_f4(input.at(x * 2 + 0, y * 2 + 0, z * 2 + 0));
                     color += unpack_f4(input.at(x * 2 + 1, y * 2 + 0, z * 2 + 0));
                     color += unpack_f4(input.at(x * 2 + 0, y * 2 + 1, z * 2 + 0));
                     color += unpack_f4(input.at(x * 2 + 1, y * 2 + 1, z * 2 + 0));
                     color += unpack_f4(input.at(x * 2 + 0, y * 2 + 0, z * 2 + 1));
                     color += unpack_f4(input.at(x * 2 + 1, y * 2 + 0, z * 2 + 1));
                     color += unpack_f4(input.at(x * 2 + 0, y * 2 + 1, z * 2 + 1));
                     color += unpack_f4(input.at(x * 2 + 1, y * 2 + 1, z * 2 + 1));

                     color *= 0.125f;

                     output.at(x, y, z) = pack(color);
                  }
               }
            }
         }
      }
   }
}

void generate_normal_maps(texture& texture, float bump_scale)
{
   if (texture.depth() > 1) {
      throw texture_error{"Can not generate normal maps for volume texture.",
                          texture_ec::generate_normal_maps_volume};
   }

   std::vector<uint8> height_map_storage;

   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      for (uint32 mip = 0; mip < texture.mip_levels(); ++mip) {
         texture_slice slice =
            texture.subresource({.array_index = array_index, .mip_level = mip}).slice(0);

         generate_normal_map(build_height_map(slice, height_map_storage), slice,
                             bump_scale);
      }
   }
}

void generate_normal_maps_highq(texture& texture, float bump_scale)
{
   if (texture.depth() > 1) {
      throw texture_error{"Can not generate normal maps for volume texture.",
                          texture_ec::generate_normal_maps_volume};
   }

   std::vector<uint8> height_map_storage;

   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      for (uint32 mip = 0; mip < texture.mip_levels(); ++mip) {
         texture_slice slice =
            texture.subresource({.array_index = array_index, .mip_level = mip}).slice(0);

         generate_normal_map_highq(build_height_map(slice, height_map_storage),
                                   slice, bump_scale);
      }
   }
}

void normalize_maps(texture& texture) noexcept
{
   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      for (uint32 mip = 0; mip < texture.mip_levels(); ++mip) {
         texture_subresource subresource =
            texture.subresource({.array_index = array_index, .mip_level = mip});

         for (uint32 z = 0; z < subresource.depth(); ++z) {
            texture_slice slice = subresource.slice(z);

            for (uint32 y = 0; y < slice.height(); ++y) {
               for (uint32 x = 0; x < slice.width(); ++x) {
                  uint32& color = slice.at(x, y);

                  float3 normal = normalize(unpack_f3(color) * 2.0f - 1.0f);

                  color &= 0xff'00'00'00;
                  color |= static_cast<uint32>(normal.x * 127.0f + 128.0f) << 16u;
                  color |= static_cast<uint32>(normal.y * 127.0f + 128.0f) << 8u;
                  color |= static_cast<uint32>(normal.z * 127.0f + 128.0f);
               }
            }
         }
      }
   }
}

void override_z_to_one(texture& texture) noexcept
{
   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      for (uint32 mip = 0; mip < texture.mip_levels(); ++mip) {
         texture_subresource subresource =
            texture.subresource({.array_index = array_index, .mip_level = mip});

         for (uint32 z = 0; z < subresource.depth(); ++z) {
            texture_slice slice = subresource.slice(z);

            for (uint32 y = 0; y < slice.height(); ++y) {
               for (uint32 x = 0; x < slice.width(); ++x) {
                  slice.at(x, y) |= 0xff;
               }
            }
         }
      }
   }
}

void apply_u_border(texture& texture, uint32 border_and_mask, uint32 border_or_mask) noexcept
{
   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      for (uint32 mip = 0; mip < texture.mip_levels(); ++mip) {
         texture_subresource subresource =
            texture.subresource({.array_index = array_index, .mip_level = mip});

         for (uint32 z = 0; z < subresource.depth(); ++z) {
            texture_slice slice = subresource.slice(z);

            for (uint32 y = 0; y < slice.height(); ++y) {
               for (uint32 x : {0u, slice.width() - 1u}) {
                  uint32& color = slice.at(x, y);

                  color &= border_and_mask;
                  color |= border_or_mask;
               }
            }
         }
      }
   }
}

void apply_v_border(texture& texture, uint32 border_and_mask, uint32 border_or_mask) noexcept
{
   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      for (uint32 mip = 0; mip < texture.mip_levels(); ++mip) {
         texture_subresource subresource =
            texture.subresource({.array_index = array_index, .mip_level = mip});

         for (uint32 z = 0; z < subresource.depth(); ++z) {
            texture_slice slice = subresource.slice(z);

            for (uint32 y : {0u, slice.height() - 1u}) {
               for (uint32 x = 0; x < slice.width(); ++x) {
                  uint32& color = slice.at(x, y);

                  color &= border_and_mask;
                  color |= border_or_mask;
               }
            }
         }
      }
   }
}

}