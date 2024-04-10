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
};

constexpr auto format_size(const texture_format format) -> std::size_t
{
   switch (format) {
   case texture_format::r8g8b8a8_unorm:
   case texture_format::r8g8b8a8_unorm_srgb:
   case texture_format::b8g8r8a8_unorm:
   case texture_format::b8g8r8a8_unorm_srgb:
      return sizeof(uint32);
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
   }

   std::terminate();
}

}
