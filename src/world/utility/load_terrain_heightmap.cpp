#include "load_terrain_heightmap.hpp"
#include "utility/com_ptr.hpp"
#include "utility/string_icompare.hpp"

#include <bit>

#include <DirectXTex.h>
#include <wil/resource.h>
#include <wincodec.h>

namespace we::world {

auto load_heightmap(const std::filesystem::path& file_path)
   -> std::variant<container::dynamic_array_2d<uint8>, container::dynamic_array_2d<uint16>>
{
   DirectX::ScratchImage image;

   if (string::iequals(file_path.extension().string(), ".tga")) {
      if (FAILED(DirectX::LoadFromTGAFile(file_path.c_str(), nullptr, image))) {
         throw terrain_map_load_error{"Failed to load image."};
      }
   }
   else if (string::iequals(file_path.extension().string(), ".png")) {
      if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      const auto cleanup = wil::scope_exit([] { CoUninitialize(); });

      utility::com_ptr<IWICImagingFactory> factory;

      if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(factory.clear_and_assign())))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      utility::com_ptr<IWICBitmapDecoder> decoder;

      if (FAILED(factory->CreateDecoderFromFilename(file_path.c_str(), nullptr, GENERIC_READ,
                                                    WICDecodeMetadataCacheOnDemand,
                                                    decoder.clear_and_assign()))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      utility::com_ptr<IWICBitmapFrameDecode> frame_decode;

      if (FAILED(decoder->GetFrame(0, frame_decode.clear_and_assign()))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      WICPixelFormatGUID loaded_format{};

      if (FAILED(frame_decode->GetPixelFormat(&loaded_format))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      DXGI_FORMAT dxgi_format = DXGI_FORMAT_UNKNOWN;

      if (loaded_format == GUID_WICPixelFormat8bppGray) {
         dxgi_format = DXGI_FORMAT_R8_UNORM;
      }
      else if (loaded_format == GUID_WICPixelFormat16bppGray)
         dxgi_format = DXGI_FORMAT_R16_UNORM;
      else {
         throw terrain_map_load_error{
            "Unsupported image format. When loading a "
            "heightmap from a .PNG file it's format "
            "must be 8-bit gray or 16-bit gray."};
      }

      uint32 width = 0;
      uint32 height = 0;

      if (FAILED(frame_decode->GetSize(&width, &height))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      if (FAILED(image.Initialize2D(dxgi_format, width, height, 1, 1))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      if (image.GetImage(0, 0, 0)->rowPitch > UINT32_MAX or
          image.GetImage(0, 0, 0)->slicePitch > UINT32_MAX) {
         throw terrain_map_load_error{"Failed to load image. Image too large."};
      }

      if (FAILED(frame_decode->CopyPixels(
             nullptr, static_cast<uint32>(image.GetImage(0, 0, 0)->rowPitch),
             static_cast<uint32>(image.GetImage(0, 0, 0)->slicePitch),
             image.GetImage(0, 0, 0)->pixels))) {
         throw terrain_map_load_error{"Failed to load image."};
      }
   }
   else {
      throw terrain_map_load_error{"Unsupported file type."};
   }

   if (image.GetMetadata().width != image.GetMetadata().height) {
      throw terrain_map_load_error{
         "Image must be a square (Width and Height must be equal)."};
   }

   if (not std::has_single_bit(image.GetMetadata().width)) {
      throw terrain_map_load_error{
         "Image width and height must be a power of 2."};
   }

   if (image.GetMetadata().width > 1024) {
      throw terrain_map_load_error{"Image width and height are too large. The "
                                   "max terrain length is 1024."};
   }

   if (image.GetMetadata().format != DXGI_FORMAT_R8_UNORM and
       image.GetMetadata().format != DXGI_FORMAT_R16_UNORM) {
      DirectX::ScratchImage converted_image;

      if (FAILED(DirectX::Convert(*image.GetImage(0, 0, 0), DXGI_FORMAT_R8_UNORM,
                                  DirectX::TEX_FILTER_FORCE_NON_WIC, 1.0f,
                                  converted_image))) {
         throw terrain_map_load_error{"Failed to convert image format."};
      }

      std::swap(converted_image, image);
   }

   if (image.GetMetadata().format == DXGI_FORMAT_R8_UNORM) {
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
   else if (image.GetMetadata().format == DXGI_FORMAT_R16_UNORM) {
      container::dynamic_array_2d<uint16> heightmap{image.GetMetadata().width,
                                                    image.GetMetadata().width};

      for (std::size_t y = 0; y < image.GetMetadata().height; ++y) {
         std::memcpy(&heightmap[{0, static_cast<std::ptrdiff_t>(y)}],
                     image.GetImage(0, 0, 0)->pixels +
                        (image.GetImage(0, 0, 0)->rowPitch * y),
                     image.GetMetadata().width * sizeof(uint16));
      }

      return heightmap;
   }
   else {
      throw terrain_map_load_error{"Unexpected image format."};
   }
}

auto load_texture_weight_map(const std::filesystem::path& file_path)
   -> container::dynamic_array_2d<uint8>
{
   DirectX::ScratchImage image;

   if (string::iequals(file_path.extension().string(), ".tga")) {
      if (FAILED(DirectX::LoadFromTGAFile(file_path.c_str(), nullptr, image))) {
         throw terrain_map_load_error{"Failed to load image."};
      }
   }
   else if (string::iequals(file_path.extension().string(), ".png")) {
      if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      const auto cleanup = wil::scope_exit([] { CoUninitialize(); });

      utility::com_ptr<IWICImagingFactory> factory;

      if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(factory.clear_and_assign())))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      utility::com_ptr<IWICBitmapDecoder> decoder;

      if (FAILED(factory->CreateDecoderFromFilename(file_path.c_str(), nullptr, GENERIC_READ,
                                                    WICDecodeMetadataCacheOnDemand,
                                                    decoder.clear_and_assign()))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      utility::com_ptr<IWICBitmapFrameDecode> frame_decode;

      if (FAILED(decoder->GetFrame(0, frame_decode.clear_and_assign()))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      WICPixelFormatGUID loaded_format{};

      if (FAILED(frame_decode->GetPixelFormat(&loaded_format))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      DXGI_FORMAT dxgi_format = DXGI_FORMAT_UNKNOWN;

      if (loaded_format == GUID_WICPixelFormat8bppGray) {
         dxgi_format = DXGI_FORMAT_R8_UNORM;
      }
      else if (loaded_format == GUID_WICPixelFormat16bppGray)
         dxgi_format = DXGI_FORMAT_R16_UNORM;
      else {
         throw terrain_map_load_error{
            "Unsupported image format. When loading a "
            "texture weight map from a .PNG file it's format "
            "must be 8-bit gray or 16-bit gray."};
      }

      uint32 width = 0;
      uint32 height = 0;

      if (FAILED(frame_decode->GetSize(&width, &height))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      if (FAILED(image.Initialize2D(dxgi_format, width, height, 1, 1))) {
         throw terrain_map_load_error{"Failed to load image."};
      }

      if (image.GetImage(0, 0, 0)->rowPitch > UINT32_MAX or
          image.GetImage(0, 0, 0)->slicePitch > UINT32_MAX) {
         throw terrain_map_load_error{"Failed to load image. Image too large."};
      }

      if (FAILED(frame_decode->CopyPixels(
             nullptr, static_cast<uint32>(image.GetImage(0, 0, 0)->rowPitch),
             static_cast<uint32>(image.GetImage(0, 0, 0)->slicePitch),
             image.GetImage(0, 0, 0)->pixels))) {
         throw terrain_map_load_error{"Failed to load image."};
      }
   }
   else {
      throw terrain_map_load_error{"Unsupported file type."};
   }

   if (image.GetMetadata().width != image.GetMetadata().height) {
      throw terrain_map_load_error{
         "Image must be a square (Width and Height must be equal)."};
   }

   if (not std::has_single_bit(image.GetMetadata().width)) {
      throw terrain_map_load_error{
         "Image width and height must be a power of 2."};
   }

   if (image.GetMetadata().width > 1024) {
      throw terrain_map_load_error{"Image width and height are too large. The "
                                   "max terrain length is 1024."};
   }

   if (image.GetMetadata().format != DXGI_FORMAT_R8_UNORM) {
      DirectX::ScratchImage converted_image;

      if (FAILED(DirectX::Convert(*image.GetImage(0, 0, 0), DXGI_FORMAT_R8_UNORM,
                                  DirectX::TEX_FILTER_FORCE_NON_WIC, 1.0f,
                                  converted_image))) {
         throw terrain_map_load_error{"Failed to convert image format."};
      }

      std::swap(converted_image, image);
   }

   container::dynamic_array_2d<uint8> weight_map{image.GetMetadata().width,
                                                 image.GetMetadata().width};

   for (std::size_t y = 0; y < image.GetMetadata().height; ++y) {
      std::memcpy(&weight_map[{0, static_cast<std::ptrdiff_t>(y)}],
                  image.GetImage(0, 0, 0)->pixels +
                     (image.GetImage(0, 0, 0)->rowPitch * y),
                  image.GetMetadata().width * sizeof(uint8));
   }

   return weight_map;
}

}
