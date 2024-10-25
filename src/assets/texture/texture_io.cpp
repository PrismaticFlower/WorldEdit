
#include "texture_io.hpp"
#include "../option_file.hpp"
#include "io/error.hpp"
#include "io/read_file.hpp"
#include "texture_transforms.hpp"
#include "utility/string_icompare.hpp"

#include <bit>
#include <cassert>
#include <stdexcept>

#include <DirectXTex.h>
#include <stb_image_resize2.h>

using namespace std::literals;

namespace we::assets::texture {

namespace {

struct texture_options {
   bool srgb = true;
   bool generate_normal_map = false;
   bool cube_map = false;
   float normal_map_scale = 1.0f;
};

struct cube_offsets {
   uint32 x = 0;
   uint32 y = 0;
};

auto load_options(io::path path) -> texture_options
{
   path += ".option";

   if (not io::exists(path)) return {};

   texture_options opts;

   for (auto& opt : parse_options(io::read_file_to_string(path))) {
      using string::iequals;

      if (iequals(opt.name, "-format"sv) or iequals(opt.name, "-forceformat"sv)) {
         if (opt.arguments.empty()) continue;

         if (const auto& format = opt.arguments[0];
             iequals(format, "bump"sv) or iequals(format, "terrain_bump"sv) or
             iequals(format, "bump_alpha"sv) or
             iequals(format, "terrain_bump_alpha"sv)) {
            opts.srgb = false;
         }
      }
      else if (iequals(opt.name, "-bumpmap"sv) or iequals(opt.name, "-hiqbumpmap"sv)) {
         opts.srgb = false;
         opts.generate_normal_map = true;
      }
      else if (iequals(opt.name, "-bumpscale"sv)) {
         if (opt.arguments.empty()) continue;

         opts.normal_map_scale = std::stof(opt.arguments[0]);
      }
      else if (iequals(opt.name, "-cubemap"sv)) {
         opts.cube_map = true;
      }
   }

   return opts;
}

auto get_mip_count(const std::size_t length) noexcept -> uint16
{
   return static_cast<uint16>(std::bit_width(length));
}

auto get_stbir_pixel_layout(const texture_format format) -> stbir_pixel_layout
{
   switch (format) {
   case texture_format::r8g8b8a8_unorm:
   case texture_format::r8g8b8a8_unorm_srgb:
      return STBIR_RGBA_NO_AW;
   case texture_format::b8g8r8a8_unorm:
   case texture_format::b8g8r8a8_unorm_srgb:
      return STBIR_BGRA_NO_AW;
   }

   std::terminate();
}

auto get_texture_format(const DXGI_FORMAT dxgi_format) -> texture_format
{
   switch (dxgi_format) {
   case DXGI_FORMAT_R8G8B8A8_UNORM:
      return texture_format::r8g8b8a8_unorm;
   case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
      return texture_format::r8g8b8a8_unorm_srgb;
   case DXGI_FORMAT_B8G8R8A8_UNORM:
      return texture_format::b8g8r8a8_unorm;
   case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
      return texture_format::b8g8r8a8_unorm_srgb;
   }

   throw std::runtime_error{"Texture has unsupported format!"};
}

void init_texture_data(texture& texture, DirectX::Image image)
{
   assert(texture.dxgi_format() == image.format);

   if (texture.flags().cube_map) {
      assert(texture.width() == image.width / 4);
      assert(texture.height() == image.height / 3);

      const cube_offsets offsets[6] = {
         {texture.width() * 2, texture.height() * 1}, // +X
         {texture.width() * 0, texture.height() * 1}, // -X
         {texture.width() * 1, texture.height() * 0}, // +Y
         {texture.width() * 1, texture.height() * 2}, // -Y
         {texture.width() * 1, texture.height() * 1}, // +Z
         {texture.width() * 3, texture.height() * 1},
      };

      for (uint32 i = 0; i < 6; ++i) {
         texture_subresource_view& texture_view =
            texture.subresource({.mip_level = 0, .array_index = i});

         const std::size_t src_row_pitch = image.rowPitch;
         const std::size_t dst_row_pitch = texture_view.row_pitch();

         const uint8* src_texels = image.pixels;
         std::byte* dst_texels = texture_view.data();

         for (uint32 y = 0; y < texture_view.height(); ++y) {
            std::memcpy(dst_texels + dst_row_pitch * y,
                        src_texels + src_row_pitch * (y + offsets[i].y) +
                           (offsets[i].x * sizeof(uint32)),
                        texture_view.width() * sizeof(uint32));
         }
      }
   }
   else {
      assert(texture.width() == image.width);
      assert(texture.height() == image.height);

      const std::size_t src_row_pitch = image.rowPitch;
      const std::size_t dst_row_pitch = texture.subresource(0).row_pitch();

      const uint8* src_texels = image.pixels;
      std::byte* dst_texels = texture.subresource(0).data();

      for (std::size_t y = 0; y < image.height; ++y) {
         std::memcpy(dst_texels + dst_row_pitch * y,
                     src_texels + src_row_pitch * y, image.width * sizeof(uint32));
      }
   }
}

void generate_mipmaps(texture& texture)
{
   const stbir_pixel_layout pixel_layout = get_stbir_pixel_layout(texture.format());

   for (uint32 array_index = 0; array_index < texture.array_size(); ++array_index) {
      for (uint32 mip = 1; mip < texture.mip_levels(); ++mip) {
         const texture_subresource_view input =
            texture.subresource({.mip_level = mip - 1, .array_index = array_index});
         texture_subresource_view output =
            texture.subresource({.mip_level = mip, .array_index = array_index});

         if (is_srgb(texture.format())) {
            stbir_resize_uint8_srgb(reinterpret_cast<const unsigned char*>(input.data()),
                                    input.width(), input.height(), input.row_pitch(),
                                    reinterpret_cast<unsigned char*>(output.data()),
                                    output.width(), output.height(),
                                    output.row_pitch(), pixel_layout);
         }
         else {
            stbir_resize_uint8_linear(reinterpret_cast<const unsigned char*>(
                                         input.data()),
                                      input.width(), input.height(),
                                      input.row_pitch(),
                                      reinterpret_cast<unsigned char*>(output.data()),
                                      output.width(), output.height(),
                                      output.row_pitch(), pixel_layout);
         }
      }
   }

#if 0
   DirectX::ScratchImage mipped_image;

   if (FAILED(DirectX::GenerateMipMaps(scratch_image.GetImages(),
                                       scratch_image.GetImageCount(),
                                       scratch_image.GetMetadata(),
                                       DirectX::TEX_FILTER_BOX | DirectX::TEX_FILTER_FORCE_NON_WIC,
                                       0, mipped_image))) {
      throw std::runtime_error{"Failed to generate mip maps."};
   }

   return mipped_image;
#endif
}

}

auto load_texture(const io::path& path) -> texture
{
   DirectX::ScratchImage scratch_image;

   if (const HRESULT hr = DirectX::LoadFromTGAFile(io::wide_path{path}.c_str(),
                                                   nullptr, scratch_image);
       FAILED(hr)) {
      throw io::open_error{"Failed to load .TGA file.",
                           hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION)
                              ? io::open_error_code::sharing_violation
                              : io::open_error_code::generic};
   }

   const auto options = load_options(path);

   if (options.srgb) {
      scratch_image.OverrideFormat(DirectX::MakeSRGB(scratch_image.GetMetadata().format));
   }

   auto metadata = scratch_image.GetMetadata();

   const uint32 width =
      static_cast<uint32>(options.cube_map ? metadata.width / 4 : metadata.width);
   const uint32 height =
      static_cast<uint32>(options.cube_map ? metadata.height / 3 : metadata.height);
   const uint16 mip_count = get_mip_count(std::max(width, height));
   const uint16 array_size = options.cube_map ? 6 : 1;

   texture texture =
      texture::init_params{.width = width,
                           .height = height,
                           .mip_levels = mip_count,
                           .array_size = array_size,
                           .format = get_texture_format(metadata.format),
                           .flags = {.cube_map = options.cube_map}};

   init_texture_data(texture, *scratch_image.GetImage(0, 0, 0));

   generate_mipmaps(texture);

   if (options.generate_normal_map and not options.cube_map) {
      texture = generate_normal_maps(texture, options.normal_map_scale);
   }

   return texture;
}

}
