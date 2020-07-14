
#include "texture_io.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <DirectXTex.h>
#include <glm/gtc/integer.hpp>
#include <range/v3/view.hpp>

namespace sk::assets::texture {

namespace {

auto get_mip_count(const std::size_t length) noexcept -> uint16
{
   return static_cast<uint16>(glm::log2(length) + 1);
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
   case DXGI_FORMAT_R16G16B16A16_UNORM:
      return texture_format::r16g16b16a16_unorm;
   case DXGI_FORMAT_R16G16B16A16_FLOAT:
      return texture_format::r16g16b16a16_float;
   case DXGI_FORMAT_R32G32B32_FLOAT:
      return texture_format::r32g32b32_float;
   case DXGI_FORMAT_R32G32B32A32_FLOAT:
      return texture_format::r32g32b32a32_float;
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

   if (FAILED(DirectX::GenerateMipMaps(*scratch_image.GetImage(0, 0, 0),
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
      glm::min(std::size_t{texture.row_pitch()}, image.rowPitch);
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

   scratch_image.OverrideFormat(DirectX::MakeSRGB(scratch_image.GetMetadata().format)); // TODO: Read .tex files and .option files to decide if an image is sRGB or not.

   if (not is_1x1x1(scratch_image)) {
      scratch_image = generate_mipmaps(std::move(scratch_image));
   }

   auto metadata = scratch_image.GetMetadata();

   texture texture =
      texture::init_params{.width = static_cast<uint32>(metadata.width),
                           .height = static_cast<uint32>(metadata.height),
                           .mip_levels = static_cast<uint16>(metadata.mipLevels),
                           .format = get_texture_format(metadata.format)};

   for (uint32 i = 0; i < texture.mip_levels(); ++i) {
      init_texture_data(texture.subresource({.mip_level = i}),
                        *scratch_image.GetImage(i, 0, 0));
   }

   return texture;
}

}