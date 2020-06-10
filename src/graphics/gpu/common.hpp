#pragma once

#include "types.hpp"

#include <optional>

#include <d3d12.h>

namespace sk::graphics::gpu {

constexpr static int render_latency = 2;

struct buffer_desc {
   uint32 alignment = 0;
   uint32 size = 0;
   D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

   operator D3D12_RESOURCE_DESC() const noexcept
   {
      return {.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
              .Alignment = alignment,
              .Width = size,
              .Height = 1,
              .DepthOrArraySize = 1,
              .MipLevels = 1,
              .SampleDesc = {1, 0},
              .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
              .Flags = flags};
   };
};

struct texture_desc {
   D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
   D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
   D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint16 width = 1;
   uint16 height = 1;
   uint16 depth = 1;
   uint16 mip_levels = 1;
   uint16 array_size = 1;

   std::optional<D3D12_CLEAR_VALUE> optimized_clear_value = std::nullopt;

   operator D3D12_RESOURCE_DESC() const noexcept
   {
      return {.Dimension = dimension,
              .Alignment = 0,
              .Width = width,
              .Height = height,
              .DepthOrArraySize =
                 dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? depth : array_size,
              .MipLevels = mip_levels,
              .Format = format,
              .SampleDesc = {1, 0},
              .Layout = layout,
              .Flags = flags};
   };
};

}
