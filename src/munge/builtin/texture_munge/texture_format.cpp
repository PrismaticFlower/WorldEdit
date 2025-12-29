#include "texture_format.hpp"
#include "error.hpp"

#include <bit>

#include <fmt/format.h>

namespace we::munge {

namespace {

constexpr uint32 D3DFMT_A8R8G8B8 = 21;
constexpr uint32 D3DFMT_R5G6B5 = 23;
constexpr uint32 D3DFMT_A1R5G5B5 = 25;
constexpr uint32 D3DFMT_A4R4G4B4 = 26;
constexpr uint32 D3DFMT_A8 = 28;
constexpr uint32 D3DFMT_L8 = 50;
constexpr uint32 D3DFMT_A8L8 = 51;
constexpr uint32 D3DFMT_A4L4 = 52;
constexpr uint32 D3DFMT_V8U8 = 60;
constexpr uint32 D3DFMT_DXT1 = 827611204;
constexpr uint32 D3DFMT_DXT3 = 861165636;
constexpr uint32 D3DFMT_DXT5 = 894720068;

}

auto get_write_format(const texture& texture, const texture_format format,
                      const texture_traits traits) -> texture_write_format
{
   const bool non_power_of_2 = not std::has_single_bit(texture.width()) or
                               not std::has_single_bit(texture.height());

   switch (format) {
   default:
   case texture_format::unspecified:
   case texture_format::unknown: {
      if (traits.is_greyscale and not traits.has_alpha) {
         return texture_write_format::l8;
      }
      else if (traits.has_alpha and not non_power_of_2) {
         return texture_write_format::dxt5;
      }
      else if (not non_power_of_2) {
         return texture_write_format::dxt1;
      }
      else {
         return texture_write_format::a8r8g8b8;
      }
   } break;
   case texture_format::meta_detail: {
      return texture_write_format::l8;
   } break;
   case texture_format::meta_bump: {
      if (not non_power_of_2) {
         return texture_write_format::dxt1;
      }
      else {
         return texture_write_format::a8r8g8b8;
      }
   } break;
   case texture_format::meta_bump_alpha: {
      if (traits.has_single_bit_alpha and not non_power_of_2) {
         return texture_write_format::dxt1;
      }
      else if (traits.has_alpha and not non_power_of_2) {
         return texture_write_format::dxt5;
      }
      else {
         return texture_write_format::a8r8g8b8;
      }
   } break;
   case texture_format::meta_compressed: {
      if (traits.is_greyscale) {
         return texture_write_format::l8;
      }
      else if (not non_power_of_2) {
         return texture_write_format::dxt1;
      }
      else {
         throw texture_error{fmt::format("Compressed formats can not be used "
                                         "with non-power of 2 textures. "
                                         "Texture size: {}x{}",
                                         texture.width(), texture.height()),
                             texture_ec::format_select_compressed_non_pow2};
      }
   } break;
   case texture_format::meta_compressed_alpha: {
      if (traits.is_greyscale and not traits.has_alpha) {
         return texture_write_format::l8;
      }
      else if (traits.has_single_bit_alpha and not non_power_of_2) {
         return texture_write_format::dxt1;
      }
      else if (not non_power_of_2) {
         return texture_write_format::dxt5;
      }
      else {
         throw texture_error{fmt::format("Compressed formats can not be used "
                                         "with non-power of 2 textures. "
                                         "Texture size: {}x{}",
                                         texture.width(), texture.height()),
                             texture_ec::format_select_compressed_alpha_non_pow2};
      }
   } break;
   case texture_format::dxt1: {
      if (non_power_of_2) {
         throw texture_error{
            fmt::format(
               "DXT/BC1 format can not be used with non-power of 2 textures. "
               "Texture size: {}x{}",
               texture.width(), texture.height()),
            texture_ec::format_select_dxt1_non_pow2};
      }

      return texture_write_format::dxt1;
   } break;
   case texture_format::dxt3: {
      if (non_power_of_2) {
         throw texture_error{
            fmt::format(
               "DXT3/BC2 format can not be used with non-power of 2 textures. "
               "Texture size: {}x{}",
               texture.width(), texture.height()),
            texture_ec::format_select_dxt3_non_pow2};
      }

      return texture_write_format::dxt3;
   } break;
   case texture_format::dxt5: {
      if (non_power_of_2) {
         throw texture_error{
            fmt::format(
               "DXT5/BC3 format can not be used with non-power of 2 textures. "
               "Texture size: {}x{}",
               texture.width(), texture.height()),
            texture_ec::format_select_dxt5_non_pow2};
      }

      return texture_write_format::dxt5;
   } break;
   case texture_format::a8r8g8b8:
      return texture_write_format::a8r8g8b8;
   case texture_format::x8r8g8b8:
      return texture_write_format::a8r8g8b8;
   case texture_format::a4r4g4b4:
      return texture_write_format::a4r4g4b4;
   case texture_format::a1r5g5b5:
      return texture_write_format::a1r5g5b5;
   case texture_format::r5g6b5:
      return texture_write_format::r5g6b5;
   case texture_format::x1r5g5b5:
      return texture_write_format::a1r5g5b5;
   case texture_format::a8l8:
      return texture_write_format::a8l8;
   case texture_format::a8:
      return texture_write_format::a8;
   case texture_format::l8:
      return texture_write_format::l8;
   case texture_format::a4l4:
      return texture_write_format::a4l4;
   case texture_format::v8u8:
      return texture_write_format::v8u8;
   }
}

auto bytes_per_texel(const texture_write_format format) noexcept -> uint32
{
   switch (format) {
   case texture_write_format::dxt1:
   case texture_write_format::dxt3:
   case texture_write_format::dxt5:
      return 0;
   case texture_write_format::a8r8g8b8:
      return 4;
   case texture_write_format::a4r4g4b4:
      return 2;
   case texture_write_format::a1r5g5b5:
      return 2;
   case texture_write_format::r5g6b5:
      return 2;
   case texture_write_format::a8l8:
      return 2;
   case texture_write_format::a8:
      return 1;
   case texture_write_format::l8:
      return 1;
   case texture_write_format::a4l4:
      return 1;
   case texture_write_format::v8u8:
      return 2;
   }

   std::unreachable();
}

auto bytes_per_block(const texture_write_format format) noexcept -> uint32
{
   switch (format) {
   case texture_write_format::dxt1:
      return 8;
   case texture_write_format::dxt3:
   case texture_write_format::dxt5:
      return 16;
   case texture_write_format::a8r8g8b8:
   case texture_write_format::a4r4g4b4:
   case texture_write_format::a1r5g5b5:
   case texture_write_format::r5g6b5:
   case texture_write_format::a8l8:
   case texture_write_format::a8:
   case texture_write_format::l8:
   case texture_write_format::a4l4:
   case texture_write_format::v8u8:
      return 0;
   }

   std::unreachable();
}

bool is_block_compressed(const texture_write_format format) noexcept
{
   switch (format) {
   case texture_write_format::dxt1:
   case texture_write_format::dxt3:
   case texture_write_format::dxt5:
      return true;
   case texture_write_format::a8r8g8b8:
   case texture_write_format::a4r4g4b4:
   case texture_write_format::a1r5g5b5:
   case texture_write_format::r5g6b5:
   case texture_write_format::a8l8:
   case texture_write_format::a8:
   case texture_write_format::l8:
   case texture_write_format::a4l4:
   case texture_write_format::v8u8:
      return false;
   }

   std::unreachable();
}

auto to_d3dformat(const texture_write_format format) noexcept -> uint32
{
   switch (format) {
   case texture_write_format::dxt1:
      return D3DFMT_DXT1;
   case texture_write_format::dxt3:
      return D3DFMT_DXT3;
   case texture_write_format::dxt5:
      return D3DFMT_DXT5;
   case texture_write_format::a8r8g8b8:
      return D3DFMT_A8R8G8B8;
   case texture_write_format::a4r4g4b4:
      return D3DFMT_A4R4G4B4;
   case texture_write_format::a1r5g5b5:
      return D3DFMT_A1R5G5B5;
   case texture_write_format::r5g6b5:
      return D3DFMT_R5G6B5;
   case texture_write_format::a8l8:
      return D3DFMT_A8L8;
   case texture_write_format::a8:
      return D3DFMT_A8;
   case texture_write_format::l8:
      return D3DFMT_L8;
   case texture_write_format::a4l4:
      return D3DFMT_A4L4;
   case texture_write_format::v8u8:
      return D3DFMT_V8U8;
   }

   std::unreachable();
}

}