#include "load_terrain_brush.hpp"

#include <DirectXTex.h>

namespace we::world {

auto load_brush(const io::path& file_path) -> container::dynamic_array_2d<uint8>
{
   DirectX::ScratchImage image;

   if (FAILED(DirectX::LoadFromTGAFile(io::wide_path{file_path}.c_str(), nullptr, image))) {
      throw brush_load_error{"Failed to load image."};
   }

   if (image.GetMetadata().format != DXGI_FORMAT_R8_UNORM) {
      DirectX::ScratchImage converted_image;

      if (FAILED(DirectX::Convert(*image.GetImage(0, 0, 0), DXGI_FORMAT_R8_UNORM,
                                  DirectX::TEX_FILTER_FORCE_NON_WIC, 1.0f,
                                  converted_image))) {
         throw brush_load_error{"Failed to convert image format."};
      }

      std::swap(converted_image, image);
   }

   if (image.GetMetadata().width % 2 != 1 or image.GetMetadata().height % 2 != 1) {
      throw brush_load_error{"Image dimensions must be odd."};
   }

   container::dynamic_array_2d<uint8> heightmap{image.GetMetadata().width,
                                                image.GetMetadata().height};

   for (std::size_t y = 0; y < image.GetMetadata().height; ++y) {
      std::memcpy(&heightmap[{0, static_cast<std::ptrdiff_t>(y)}],
                  image.GetImage(0, 0, 0)->pixels +
                     (image.GetImage(0, 0, 0)->rowPitch * y),
                  image.GetMetadata().width * sizeof(uint8));
   }

   return heightmap;
}

}