#pragma once

#include "device.hpp"
#include "texture.hpp"

namespace sk::graphics::gpu {

struct depth_stencil_texture : private texture {
   struct depth_stencil_texture_init {
      DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
      uint16 width = 1;
      uint16 height = 1;

      D3D12_CLEAR_VALUE optimized_clear_value = {.Format = format,
                                                 .DepthStencil = {.Depth = 1.0f,
                                                                  .Stencil = 0x0}};

      operator texture_desc() const noexcept
      {
         return {.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                 .flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
                          D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE,
                 .format = format,
                 .width = width,
                 .height = height,
                 .optimized_clear_value = optimized_clear_value};
      };
   };

   depth_stencil_texture() = default;

   depth_stencil_texture(device& device, const depth_stencil_texture_init init_params,
                         const D3D12_RESOURCE_STATES initial_resource_state)
      : texture{device.create_texture(init_params, initial_resource_state)}
   {
      depth_stencil_view = device.dsv_descriptor_heap.allocate();
      device.device_d3d->CreateDepthStencilView(resource(), nullptr, depth_stencil_view);
   }

   depth_stencil_texture(const depth_stencil_texture&) noexcept = delete;
   auto operator=(const depth_stencil_texture&) noexcept -> texture& = delete;

   depth_stencil_texture(depth_stencil_texture&& other) noexcept
   {
      this->swap(other);
   }

   auto operator=(depth_stencil_texture&& other) noexcept -> texture&
   {
      depth_stencil_texture discarded;

      discarded.swap(*this);
      this->swap(other);

      return *this;
   }

   ~depth_stencil_texture()
   {
      if (alive()) {
         parent_device()->dsv_descriptor_heap.free(depth_stencil_view);
      }
   }

   void swap(depth_stencil_texture& other) noexcept
   {
      using std::swap;

      texture::swap(other);
      swap(this->depth_stencil_view, other.depth_stencil_view);
   }

   using texture::alive;

   using texture::parent_device;
   using texture::resource;

   using texture::array_size;
   using texture::depth;
   using texture::format;
   using texture::height;
   using texture::mip_levels;
   using texture::width;

   D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_view{};
};

}
