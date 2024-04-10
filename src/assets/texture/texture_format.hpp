#pragma once

#include "types.hpp"

#include <cstddef>
#include <exception>
#include <span>

#include <dxgiformat.h>

namespace we::assets::texture {

enum class texture_format {
   r8g8b8a8_unorm,
   r8g8b8a8_unorm_srgb,
   b8g8r8a8_unorm,
   b8g8r8a8_unorm_srgb,
   r16g16b16a16_unorm,
};

constexpr auto format_size(const texture_format format) -> std::size_t
{
   switch (format) {
   case texture_format::r8g8b8a8_unorm:
   case texture_format::r8g8b8a8_unorm_srgb:
   case texture_format::b8g8r8a8_unorm:
   case texture_format::b8g8r8a8_unorm_srgb:
      return sizeof(uint32);
   case texture_format::r16g16b16a16_unorm:
      return sizeof(uint64);
   }

   std::terminate();
}

constexpr auto to_dxgi_format(const texture_format format) -> DXGI_FORMAT
{
   switch (format) {
   case texture_format::r8g8b8a8_unorm:
      return DXGI_FORMAT_R8G8B8A8_UNORM;
   case texture_format::r8g8b8a8_unorm_srgb:
      return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
   case texture_format::b8g8r8a8_unorm:
      return DXGI_FORMAT_B8G8R8A8_UNORM;
   case texture_format::b8g8r8a8_unorm_srgb:
      return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
   case texture_format::r16g16b16a16_unorm:
      return DXGI_FORMAT_R16G16B16A16_UNORM;
   }

   std::terminate();
}

}
