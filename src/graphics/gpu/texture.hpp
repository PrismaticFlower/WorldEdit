#pragma once

#include "device.hpp"
#include "hresult_error.hpp"
#include "types.hpp"

#include <cassert>
#include <optional>
#include <utility>

#include <gsl/gsl>
#include <object_ptr.hpp>

namespace sk::graphics::gpu {

struct texture {
   struct texture_init {
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

   texture() = default;

   texture(device& device, const texture_init init_params,
           const D3D12_HEAP_TYPE heap_type,
           const D3D12_RESOURCE_STATES initial_resource_state)
   {
      const D3D12_HEAP_PROPERTIES heap_properties{.Type = heap_type,
                                                  .CPUPageProperty =
                                                     D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                  .MemoryPoolPreference =
                                                     D3D12_MEMORY_POOL_UNKNOWN};
      const D3D12_RESOURCE_DESC desc = init_params;

      const D3D12_CLEAR_VALUE* optimized_clear_value =
         init_params.optimized_clear_value
            ? &init_params.optimized_clear_value.value()
            : nullptr;

      throw_if_failed(
         device.device_d3d->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE,
                                                    &desc, initial_resource_state,
                                                    optimized_clear_value,
                                                    IID_PPV_ARGS(&resource)));

      parent_device = &device;
      resource_state = initial_resource_state;
      format = init_params.format;
      width = init_params.width;
      height = init_params.height;
      depth = init_params.depth;
      mip_levels = init_params.mip_levels;
      array_size = init_params.array_size;
   }

   texture(const texture&) noexcept = delete;
   auto operator=(const texture&) noexcept -> texture& = delete;

   texture(texture&& other) noexcept
   {
      this->swap(other);
   }

   auto operator=(texture&& other) noexcept -> texture&
   {
      texture discarded;

      discarded.swap(*this);
      this->swap(other);

      return *this;
   }

   ~texture()
   {
      if (resource) {
         assert(parent_device);

         parent_device->deferred_destroy_resource(*this);
      }
   }

   void swap(texture& other) noexcept
   {
      using std::swap;

      swap(this->parent_device, other.parent_device);
      swap(this->resource, other.resource);
      swap(this->resource_state, other.resource_state);
      swap(this->format, other.format);
      swap(this->width, other.width);
      swap(this->height, other.height);
      swap(this->depth, other.depth);
      swap(this->mip_levels, other.mip_levels);
      swap(this->array_size, other.array_size);
   }

   bool alive() const noexcept
   {
      return resource;
   }

   explicit operator bool() const noexcept
   {
      return alive();
   }

   jss::object_ptr<device> parent_device = nullptr;
   gsl::owner<ID3D12Resource*> resource = nullptr;
   D3D12_RESOURCE_STATES resource_state = D3D12_RESOURCE_STATE_COMMON;

   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint16 width = 0;
   uint16 height = 0;
   uint16 depth = 0;
   uint16 mip_levels = 0;
   uint16 array_size = 0;
};

}
