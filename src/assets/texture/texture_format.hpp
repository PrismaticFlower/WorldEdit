#pragma once

#include "types.hpp"

#include <cstddef>
#include <exception>
#include <span>

#include <dxgiformat.h>

namespace sk::assets::texture {

enum class texture_format {
   r8g8b8a8_unorm,
   r8g8b8a8_unorm_srgb,
   b8g8r8a8_unorm,
   b8g8r8a8_unorm_srgb,
   r16g16b16a16_unorm,
   r16g16b16a16_float,
   r32g32b32_float,
   r32g32b32a32_float
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
   case texture_format::r16g16b16a16_float:
      return sizeof(uint64);
   case texture_format::r32g32b32_float:
      return sizeof(float3);
   case texture_format::r32g32b32a32_float:
      return sizeof(float4);
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
   case texture_format::r16g16b16a16_float:
      return DXGI_FORMAT_R16G16B16A16_FLOAT;
   case texture_format::r32g32b32_float:
      return DXGI_FORMAT_R32G32B32_FLOAT;
   case texture_format::r32g32b32a32_float:
      return DXGI_FORMAT_R32G32B32A32_FLOAT;
   }

   std::terminate();
}

auto load_texel(const texture_format format, const uint32 x, const uint32 y,
                const std::span<const std::byte> data, const uint32 width,
                const uint32 height, const uint32 row_pitch) -> float4;

void store_texel(const float4 value, const texture_format format,
                 const uint32 x, const uint32 y, const std::span<std::byte> data,
                 const uint32 width, const uint32 height, const uint32 row_pitch);

}