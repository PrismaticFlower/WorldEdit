
#include "texture_io.hpp"
#include "../option_file.hpp"
#include "io/read_file.hpp"
#include "texture_transforms.hpp"
#include "utility/string_icompare.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <DirectXTex.h>

using namespace std::literals;

namespace we::assets::texture {

namespace {

struct texture_options {
   bool srgb = true;
   bool generate_normal_map = false;
   bool cube_map = false;
   float normal_map_scale = 1.0f;
};

auto load_options(std::filesystem::path path) -> texture_options
{
   path += L".option"s;

   if (not std::filesystem::exists(path)) return {};

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
   return static_cast<uint16>(std::log2(length) + 1);
}

auto fold_cube_map(DirectX::ScratchImage image) -> DirectX::ScratchImage
{
   if ((image.GetMetadata().width % 4) != 0 or (image.GetMetadata().height % 3) != 0) {
      throw std::runtime_error{"Invalid cube map dimensions!"};
   }

   const auto width = (image.GetMetadata().width / 4);
   const auto height = (image.GetMetadata().height / 3);

   DirectX::ScratchImage cube_map;
   cube_map.InitializeCube(image.GetMetadata().format, width, height, 1, 1);

   // +X
   DirectX::CopyRectangle(*image.GetImage(0, 0, 0), {width * 2, height, width, height},
                          *cube_map.GetImage(0, 0, 0),
                          DirectX::TEX_FILTER_FORCE_NON_WIC, 0, 0);

   // -X
   DirectX::CopyRectangle(*image.GetImage(0, 0, 0), {0, height, width, height},
                          *cube_map.GetImage(0, 1, 0),
                          DirectX::TEX_FILTER_FORCE_NON_WIC, 0, 0);

   // +Y
   DirectX::CopyRectangle(*image.GetImage(0, 0, 0), {width, 0, width, height},
                          *cube_map.GetImage(0, 2, 0),
                          DirectX::TEX_FILTER_FORCE_NON_WIC, 0, 0);

   // -Y
   DirectX::CopyRectangle(*image.GetImage(0, 0, 0), {width, height * 2, width, height},
                          *cube_map.GetImage(0, 3, 0),
                          DirectX::TEX_FILTER_FORCE_NON_WIC, 0, 0);

   // +Z
   DirectX::CopyRectangle(*image.GetImage(0, 0, 0), {width, height, width, height},
                          *cube_map.GetImage(0, 4, 0),
                          DirectX::TEX_FILTER_FORCE_NON_WIC, 0, 0);

   // +Z
   DirectX::CopyRectangle(*image.GetImage(0, 0, 0), {width * 3, height, width, height},
                          *cube_map.GetImage(0, 5, 0),
                          DirectX::TEX_FILTER_FORCE_NON_WIC, 0, 0);

   return cube_map;
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

bool is_1x1x1(const DirectX::ScratchImage& image) noexcept
{
   return image.GetMetadata().width == 1 and image.GetMetadata().height == 1 and
          image.GetMetadata().depth == 1;
}

auto generate_mipmaps(DirectX::ScratchImage scratch_image) -> DirectX::ScratchImage
{
   DirectX::ScratchImage mipped_image;

   if (FAILED(DirectX::GenerateMipMaps(scratch_image.GetImages(),
                                       scratch_image.GetImageCount(),
                                       scratch_image.GetMetadata(),
                                       DirectX::TEX_FILTER_BOX | DirectX::TEX_FILTER_FORCE_NON_WIC,
                                       0, mipped_image))) {
      throw std::runtime_error{"Failed to generate mip maps."};
   }

   return mipped_image;
}

void init_texture_data(texture_subresource_view texture, DirectX::Image image)
{
   assert(texture.dxgi_format() == image.format);
   assert(texture.width() == image.width);
   assert(texture.height() == image.height);

   const std::size_t row_pitch =
      std::min(std::size_t{texture.row_pitch()}, image.rowPitch);
   const std::byte* src_pixels = reinterpret_cast<const std::byte*>(image.pixels);

   for (std::size_t y = 0; y < image.height; ++y) {
      std::copy_n(src_pixels + (y * image.rowPitch), row_pitch,
                  texture.data() + (y * texture.row_pitch()));
   }
}

}

auto load_texture(const std::filesystem::path& path) -> texture
{
   DirectX::ScratchImage scratch_image;

   if (FAILED(DirectX::LoadFromTGAFile(path.c_str(), nullptr, scratch_image))) {
      throw std::runtime_error{"Failed to load .TGA file."};
   }

   const auto options = load_options(path);

   if (options.srgb) {
      scratch_image.OverrideFormat(DirectX::MakeSRGB(scratch_image.GetMetadata().format));
   }

   if (options.cube_map) {
      scratch_image = fold_cube_map(std::move(scratch_image));
   }

   if (not is_1x1x1(scratch_image)) {
      scratch_image = generate_mipmaps(std::move(scratch_image));
   }

   auto metadata = scratch_image.GetMetadata();

   texture texture =
      texture::init_params{.width = static_cast<uint32>(metadata.width),
                           .height = static_cast<uint32>(metadata.height),
                           .mip_levels = static_cast<uint16>(metadata.mipLevels),
                           .array_size = static_cast<uint16>(metadata.arraySize),
                           .format = get_texture_format(metadata.format),
                           .flags = {.cube_map = options.cube_map}};

   for (uint32 item = 0; item < texture.array_size(); ++item) {
      for (uint32 mip = 0; mip < texture.mip_levels(); ++mip) {
         init_texture_data(texture.subresource({.mip_level = mip, .array_index = item}),
                           *scratch_image.GetImage(mip, item, 0));
      }
   }

   if (options.generate_normal_map and not options.cube_map) {
      texture = generate_normal_maps(texture, options.normal_map_scale);
   }

   return texture;
}

}
