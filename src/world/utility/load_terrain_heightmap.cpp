#include "load_terrain_heightmap.hpp"
#include "utility/string_icompare.hpp"

#include <bit>

#include <DirectXTex.h>

namespace we::world {

auto load_heightmap(const std::filesystem::path& file_path)
   -> container::dynamic_array_2d<uint8>
{
   DirectX::ScratchImage image;

   if (FAILED(DirectX::LoadFromTGAFile(file_path.c_str(), nullptr, image))) {
      throw heightmap_load_error{"Failed to load image."};
   }

   if (image.GetMetadata().width != image.GetMetadata().height) {
      throw heightmap_load_error{
         "Image must be a square (Width and Height must be equal)."};
   }

   if (not std::has_single_bit(image.GetMetadata().width)) {
      throw heightmap_load_error{
         "Image width and height must be a power of 2."};
   }

   if (image.GetMetadata().width > 1024) {
      throw heightmap_load_error{"Image width and height are too large. The "
                                 "max terrain length is 1024."};
   }

   if (image.GetMetadata().format != DXGI_FORMAT_R8_UNORM) {
      DirectX::ScratchImage converted_image;

      if (FAILED(DirectX::Convert(*image.GetImage(0, 0, 0), DXGI_FORMAT_R8_UNORM,
                                  DirectX::TEX_FILTER_FORCE_NON_WIC, 1.0f,
                                  converted_image))) {
         throw heightmap_load_error{"Failed to convert image format."};
      }

      std::swap(converted_image, image);
   }

   container::dynamic_array_2d<uint8> heightmap{image.GetMetadata().width,
                                                image.GetMetadata().width};

   for (std::size_t y = 0; y < image.GetMetadata().height; ++y) {
      std::memcpy(&heightmap[{0, static_cast<std::ptrdiff_t>(y)}],
                  image.GetImage(0, 0, 0)->pixels +
                     (image.GetImage(0, 0, 0)->rowPitch * y),
                  image.GetMetadata().width * sizeof(uint8));
   }

   return heightmap;
}

}
