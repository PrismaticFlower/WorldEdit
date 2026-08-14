#include "export_terrain_map.hpp"

#include "utility/com_ptr.hpp"

#include <stdexcept>

#include <wil/resource.h>
#include <wincodec.h>

namespace we::world {

namespace {

template<typename T>
void export_map_png(const io::path& path, GUID pixel_format,
                    container::dynamic_array_2d<T>& map)
{
   if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
      throw std::runtime_error{"Failed to initialize COM."};
   }

   const auto cleanup = wil::scope_exit([] { CoUninitialize(); });

   utility::com_ptr<IWICImagingFactory> factory;

   if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS(factory.clear_and_assign())))) {
      throw std::runtime_error{"Failed to initialize WIC."};
   }

   utility::com_ptr<IWICBitmapEncoder> encoder;

   if (FAILED(factory->CreateEncoder(GUID_ContainerFormatPng, &GUID_VendorMicrosoft,
                                     encoder.clear_and_assign()))) {
      throw std::runtime_error{"Failed to create WIC encoder."};
   }

   utility::com_ptr<IWICStream> stream;

   if (FAILED(factory->CreateStream(stream.clear_and_assign()))) {
      throw std::runtime_error{"Failed to create WIC stream."};
   }

   if (FAILED(stream->InitializeFromFilename(io::wide_path{path}.c_str(), GENERIC_WRITE))) {
      throw std::runtime_error{"Failed to open file for writing."};
   }

   if (FAILED(encoder->Initialize(stream.get(), WICBitmapEncoderNoCache))) {
      throw std::runtime_error{"Failed to initialize WIC encoder."};
   }

   utility::com_ptr<IWICBitmapFrameEncode> frame_encode;

   if (FAILED(encoder->CreateNewFrame(frame_encode.clear_and_assign(), nullptr))) {
      throw std::runtime_error{"Failed to create WIC frame encode."};
   }

   if (FAILED(frame_encode->Initialize(nullptr))) {
      throw std::runtime_error{"Failed to initialize WIC frame encode."};
   }

   if (FAILED(frame_encode->SetPixelFormat(&pixel_format))) {
      throw std::runtime_error{"Failed to set WIC pixel format."};
   }

   if (FAILED(frame_encode->SetSize(static_cast<UINT>(map.width()),
                                    static_cast<UINT>(map.height())))) {
      throw std::runtime_error{"Failed to set WIC output size."};
   }

   if (FAILED(frame_encode->WritePixels(static_cast<UINT>(map.height()),
                                        static_cast<UINT>(map.width() * sizeof(T)),
                                        static_cast<UINT>(
                                           map.width() * map.height() * sizeof(T)),
                                        reinterpret_cast<BYTE*>(map.data())))) {
      throw std::runtime_error{"Failed to write pixels to WIC target."};
   }

   if (FAILED(frame_encode->Commit())) {
      throw std::runtime_error{"Failed to commit WIC frame encode."};
   }

   if (FAILED(encoder->Commit())) {
      throw std::runtime_error{"Failed to commit WIC encode."};
   }
}

}

void export_height_map(const io::path& path, const world& world)
{
   container::dynamic_array_2d<uint16> height_map = {
      world.terrain.height_map.width(),
      world.terrain.height_map.height(),
   };

   for (std::ptrdiff_t y = 0; y < height_map.s_height(); ++y) {
      for (std::ptrdiff_t x = 0; x < height_map.s_height(); ++x) {
         height_map[{x, y}] =
            static_cast<uint16>(world.terrain.height_map[{x, y}] + 32768);
      }
   }

   export_map_png(path, GUID_WICPixelFormat16bppGray, height_map);
}

void export_texture_weight_map(const io::path& path,
                               const container::dynamic_array_2d<uint8>& map)
{
   container::dynamic_array_2d<uint8> writable_map = map;

   export_map_png(path, GUID_WICPixelFormat8bppGray, writable_map);
}

void export_color_map(const io::path& path,
                      const container::dynamic_array_2d<uint32>& map)
{
   container::dynamic_array_2d<uint32> writable_map = map;

   export_map_png(path, GUID_WICPixelFormat32bppBGRA, writable_map);
}

}