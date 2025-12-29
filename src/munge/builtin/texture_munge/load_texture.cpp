#include "load_texture.hpp"

#include "error.hpp"

#include <bit>
#include <system_error>

#include <DirectXTex.h>

#include <fmt/format.h>

namespace we::munge {

namespace {

// These limits are based on two things.
//
// 1. What could actually fit into an .lvl, to the best of the community's knowledge 2GiB is the limit for an .lvl fire.
// 2. Direct3D limits. The stock game uses D3D9 which lets the GPU driver set the limit but D3D FL_11 has a limit of 16384x16384 and
//    Nvidia drivers at least report this limit to D3D9 as well.

const uint32 max_texture_dimension = 0x4000;
const uint32 max_cube_face_texture_dimension = 0x1000;
const uint32 max_volume_texture_dimension = 0x200;

auto operator&=(texture_traits& l, const texture_traits& r) noexcept -> texture_traits&
{
   l.is_greyscale &= r.is_greyscale;
   l.has_single_bit_alpha &= r.has_single_bit_alpha;
   l.has_alpha &= r.has_alpha;

   return l;
}

void copy_r8(const DirectX::Image& image, texture_slice slice) noexcept
{
   assert(image.width == slice.width());
   assert(image.height == slice.height());
   assert(image.format == DXGI_FORMAT_R8_UNORM);

   for (std::size_t y = 0; y < slice.height(); ++y) {
      for (std::size_t x = 0; x < slice.width(); ++x) {
         const uint8 value = image.pixels[y * image.rowPitch + x];

         uint32 packed = 0;

         packed |= value;
         packed |= value << 8;
         packed |= value << 16;
         packed |= 0xff'00'00'00u;

         slice.at(x, y) = packed;
      }
   }
}

auto copy_b5g5r5a1(const DirectX::Image& image, texture_slice slice) noexcept -> texture_traits
{
   assert(image.width == slice.width());
   assert(image.height == slice.height());
   assert(image.format == DXGI_FORMAT_B5G5R5A1_UNORM);

   uint32 alpha_mask = 0x1;

   bool greyscale = true;

   for (std::size_t y = 0; y < slice.height(); ++y) {
      for (std::size_t x = 0; x < slice.width(); ++x) {
         uint16 value = 0;

         std::memcpy(&value, &image.pixels[y * image.rowPitch + x * sizeof(uint16)],
                     sizeof(uint16));

         const uint32 blue_5 = value & 0x00'1fu;
         const uint32 green_5 = (value & 0x03'e0) >> 5u;
         const uint32 red_5 = (value & 0x7c'00) >> 10u;
         const uint32 alpha_bit = (value & 0x80'00) >> 15u;

         alpha_mask &= alpha_bit;
         greyscale &= (blue_5 == green_5) & (green_5 == red_5);

         constexpr float div_mul = (1.0f / 31.0f) * 255.0f;

         const uint32 blue = static_cast<uint32>(blue_5 * div_mul + 0.5f);
         const uint32 green = static_cast<uint32>(green_5 * div_mul + 0.5f);
         const uint32 red = static_cast<uint32>(red_5 * div_mul + 0.5f);

         uint32 packed = 0;

         packed |= blue;
         packed |= green << 8;
         packed |= red << 16;
         packed |= alpha_bit ? 0xff'00'00'00u : 0x0;

         slice.at(x, y) = packed;
      }
   }

   const bool has_alpha = alpha_mask != 0x1;

   return {
      .is_greyscale = greyscale,
      .has_single_bit_alpha = has_alpha,
      .has_alpha = has_alpha,
   };
}

auto copy_b8g8r8a8(const DirectX::Image& image, texture_slice slice) noexcept -> texture_traits
{
   assert(image.width == slice.width());
   assert(image.height == slice.height());
   assert(image.format == DXGI_FORMAT_B8G8R8A8_UNORM);

   uint32 alpha_mask = 0xff'00'00'00u;

   bool greyscale = true;
   bool single_bit_alpha = true;

   for (std::size_t y = 0; y < slice.height(); ++y) {
      for (std::size_t x = 0; x < slice.width(); ++x) {
         uint32 value = 0;

         std::memcpy(&value, &image.pixels[y * image.rowPitch + x * sizeof(uint32)],
                     sizeof(uint32));

         {
            const uint32 blue = value & 0x00'00'00'ffu;
            const uint32 green = (value & 0x00'00'ff'00u) >> 8u;
            const uint32 red = (value & 0x00'ff'00'00u) >> 16u;

            greyscale &= (blue == green) & (green == red);
         }

         {
            const uint32 alpha = value & 0xff'00'00'00u;

            single_bit_alpha &= (alpha == 0xff'00'00'00u) | (alpha == 0x00'00'00'00u);
         }

         alpha_mask &= value;

         slice.at(x, y) = value;
      }
   }

   const bool has_alpha = alpha_mask != 0xff'00'00'00u;

   return {
      .is_greyscale = greyscale,
      .has_single_bit_alpha = has_alpha and single_bit_alpha,
      .has_alpha = has_alpha,
   };
}

auto copy_b8g8r8x8(const DirectX::Image& image, texture_slice slice) noexcept -> texture_traits
{
   assert(image.width == slice.width());
   assert(image.height == slice.height());
   assert(image.format == DXGI_FORMAT_B8G8R8X8_UNORM);

   bool greyscale = true;

   for (std::size_t y = 0; y < slice.height(); ++y) {
      for (std::size_t x = 0; x < slice.width(); ++x) {
         uint32 value = 0;

         std::memcpy(&value, &image.pixels[y * image.rowPitch + x * sizeof(uint32)],
                     sizeof(uint32));

         {
            const uint32 blue = value & 0x00'00'00'ffu;
            const uint32 green = (value & 0x00'00'ff'00u) >> 8u;
            const uint32 red = (value & 0x00'ff'00'00u) >> 16u;

            greyscale &= (blue == green) & (green == red);
         }

         slice.at(x, y) = value | 0xff'00'00'00u;
      }
   }

   return {
      .is_greyscale = greyscale,
      .has_single_bit_alpha = false,
      .has_alpha = false,
   };
}

auto load_texture_2d(const DirectX::Image& image, const load_texture_info& info)
   -> load_texture_result
{
   if (std::max(image.width, image.height) > max_texture_dimension) {
      throw texture_error{
         fmt::format("Texture size of {0}x{1} is too large. Max: {2}x{2}",
                     image.width, image.height, max_texture_dimension),
         texture_ec::tga_load_too_large_2d};
   }

   load_texture_result result = {
      .texture = texture::init_params{.width = static_cast<uint32>(image.width),
                                      .height = static_cast<uint32>(image.height),
                                      .mip_levels = info.mip_levels},
   };

   switch (image.format) {
   case DXGI_FORMAT_R8_UNORM: {
      result.traits.is_greyscale = true;

      copy_r8(image, result.texture.subresource({.mip_level = 0}).slice(0));
   } break;
   case DXGI_FORMAT_B5G5R5A1_UNORM: {
      result.traits =
         copy_b5g5r5a1(image, result.texture.subresource({.mip_level = 0}).slice(0));
   } break;
   case DXGI_FORMAT_B8G8R8A8_UNORM: {
      result.traits =
         copy_b8g8r8a8(image, result.texture.subresource({.mip_level = 0}).slice(0));
   } break;
   case DXGI_FORMAT_B8G8R8X8_UNORM: {
      result.traits =
         copy_b8g8r8x8(image, result.texture.subresource({.mip_level = 0}).slice(0));
   } break;
   default: {
      throw texture_error{"Unexpected texture format!",
                          texture_ec::tga_load_unexpected_format};
   } break;
   }

   return result;
}

auto load_texture_cube(const DirectX::Image& image, const load_texture_info& info)
   -> load_texture_result
{
   if (image.width > max_cube_face_texture_dimension * 4 or
       image.height > max_cube_face_texture_dimension * 3) {
      throw texture_error{
         fmt::format("Texture size of {}x{} is too large. Max: {}x{}", image.width,
                     image.height, max_cube_face_texture_dimension * 4,
                     max_cube_face_texture_dimension * 3),
         texture_ec::tga_load_too_large_cube};
   }

   if (image.width % 4 or image.height % 3) {
      throw texture_error{
         fmt::format("Texture size of {}x{} has incorrect aspect ratio "
                     "for use as a cubemap!",
                     image.width, image.height),
         texture_ec::tga_load_bad_cube_source_aspect_ratio};
   }

   const uint32 width = static_cast<uint32>(image.width / 4);
   const uint32 height = static_cast<uint32>(image.height / 3);

   load_texture_result result = {
      .texture = texture::init_params{.width = width,
                                      .height = height,
                                      .mip_levels = info.mip_levels,
                                      .array_size = 6},
      .traits =
         {
            .is_greyscale = true,
            .has_single_bit_alpha = true,
            .has_alpha = true,
         },
   };

   const std::array<std::array<uint32, 2>, 6> offsets = {{
      {width * 2, height * 1}, // X+
      {width * 0, height * 1}, // X-
      {width * 1, height * 0}, // Y+
      {width * 1, height * 2}, // Y-
      {width * 1, height * 1}, // Z+
      {width * 3, height * 1}, // Z-
   }};

   for (uint32 face = 0; face < 6; ++face) {
      switch (image.format) {
      case DXGI_FORMAT_R8_UNORM: {
         result.traits = {.is_greyscale = true};

         DirectX::Image image_slice = image;

         image_slice.width = width;
         image_slice.height = height;
         image_slice.pixels += offsets[face][1] * image_slice.rowPitch +
                               offsets[face][0] * sizeof(uint8);

         copy_r8(image_slice, result.texture
                                 .subresource({.array_index = face, .mip_level = 0})
                                 .slice(0));
      } break;
      case DXGI_FORMAT_B5G5R5A1_UNORM: {
         DirectX::Image image_slice = image;

         image_slice.width = width;
         image_slice.height = height;
         image_slice.pixels += offsets[face][1] * image_slice.rowPitch +
                               offsets[face][0] * sizeof(uint16);

         result.traits &=
            copy_b5g5r5a1(image_slice,
                          result.texture
                             .subresource({.array_index = face, .mip_level = 0})
                             .slice(0));
      } break;
      case DXGI_FORMAT_B8G8R8A8_UNORM: {
         DirectX::Image image_slice = image;

         image_slice.width = width;
         image_slice.height = height;
         image_slice.pixels += offsets[face][1] * image_slice.rowPitch +
                               offsets[face][0] * sizeof(uint32);

         result.traits &=
            copy_b8g8r8a8(image_slice,
                          result.texture
                             .subresource({.array_index = face, .mip_level = 0})
                             .slice(0));
      } break;
      case DXGI_FORMAT_B8G8R8X8_UNORM: {
         DirectX::Image image_slice = image;

         image_slice.width = width;
         image_slice.height = height;
         image_slice.pixels += offsets[face][1] * image_slice.rowPitch +
                               offsets[face][0] * sizeof(uint32);

         result.traits &=
            copy_b8g8r8x8(image_slice,
                          result.texture
                             .subresource({.array_index = face, .mip_level = 0})
                             .slice(0));
      } break;
      default: {
         throw texture_error{"Unexpected texture format!",
                             texture_ec::tga_load_unexpected_format};
      } break;
      }
   }

   return result;
}

auto load_texture_volume(const DirectX::Image& image, const load_texture_info& info)
   -> load_texture_result
{
   if (image.width > max_volume_texture_dimension or
       image.height > max_volume_texture_dimension * max_volume_texture_dimension) {
      throw texture_error{
         fmt::format("Texture size of {}x{} is too large. Max: {}x{}",
                     image.width, image.height, max_volume_texture_dimension,
                     max_volume_texture_dimension * max_volume_texture_dimension),
         texture_ec::tga_load_too_large_volume};
   }

   if (info.depth == 0) {
      throw texture_error{"Volume depth can not be 0.",
                          texture_ec::tga_load_bad_volume_depth};
   }

   if (info.depth > max_volume_texture_dimension) {
      throw texture_error{
         fmt::format("Requested volume depth of {} is too large. Max: {}",
                     info.depth, max_volume_texture_dimension),
         texture_ec::tga_load_bad_volume_depth};
   }

   if (image.height % info.depth) {
      throw texture_error{
         fmt::format(
            "Texture height {} is not divisible by requested depth of {}!",
            image.height, info.depth),
         texture_ec::tga_load_bad_volume_height};
   }

   const uint32 width = static_cast<uint32>(image.width);
   const uint32 height = static_cast<uint32>(image.height / info.depth);
   const uint32 depth = info.depth;

   load_texture_result result = {
      .texture = texture::init_params{.width = width,
                                      .height = height,
                                      .depth = depth,
                                      .mip_levels = info.mip_levels},
      .traits =
         {
            .is_greyscale = true,
            .has_single_bit_alpha = true,
            .has_alpha = true,
         },
   };

   for (uint32 z = 0; z < depth; ++z) {
      DirectX::Image image_slice = image;

      image_slice.width = width;
      image_slice.height = height;
      image_slice.pixels += z * depth * image_slice.rowPitch;

      switch (image.format) {
      case DXGI_FORMAT_R8_UNORM: {
         result.traits = {.is_greyscale = true};

         copy_r8(image_slice, result.texture.subresource({.mip_level = 0}).slice(z));
      } break;
      case DXGI_FORMAT_B5G5R5A1_UNORM: {
         result.traits &=
            copy_b5g5r5a1(image_slice,
                          result.texture.subresource({.mip_level = 0}).slice(z));
      } break;
      case DXGI_FORMAT_B8G8R8A8_UNORM: {
         result.traits &=
            copy_b8g8r8a8(image_slice,
                          result.texture.subresource({.mip_level = 0}).slice(z));
      } break;
      case DXGI_FORMAT_B8G8R8X8_UNORM: {
         result.traits &=
            copy_b8g8r8x8(image_slice,
                          result.texture.subresource({.mip_level = 0}).slice(z));
      } break;
      default: {
         throw texture_error{"Unexpected texture format!",
                             texture_ec::tga_load_unexpected_format};
      } break;
      }
   }

   return result;
}

}

auto load_texture(const io::path& path, const load_texture_info& info) -> load_texture_result
{
   DirectX::ScratchImage scratch_image;

   if (const HRESULT hr =
          DirectX::LoadFromTGAFile(io::wide_path{path}.c_str(),
                                   DirectX::TGA_FLAGS_BGR |
                                      DirectX::TGA_FLAGS_ALLOW_ALL_ZERO_ALPHA |
                                      DirectX::TGA_FLAGS_IGNORE_SRGB,
                                   nullptr, scratch_image);
       FAILED(hr)) {
      throw texture_error{std::system_category().default_error_condition(hr).message(),
                          texture_ec::tga_load_fail};
   }

   if (scratch_image.GetImageCount() < 1) {
      throw texture_error{"Unexpected image count from DirectXTex",
                          texture_ec::tga_load_fail};
   }

   const DirectX::TexMetadata metadata = scratch_image.GetMetadata();

   switch (info.type) {
   case texture_type::_2d: {
      return load_texture_2d(*scratch_image.GetImage(0, 0, 0), info);
   } break;
   case texture_type::cube: {
      return load_texture_cube(*scratch_image.GetImage(0, 0, 0), info);

   } break;
   case texture_type::volume: {
      return load_texture_volume(*scratch_image.GetImage(0, 0, 0), info);
   } break;
   }

   std::unreachable();
}

}