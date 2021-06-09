#pragma once

#include "device.hpp"
#include "texture.hpp"

namespace we::graphics::gpu {

struct depth_stencil_texture : private texture {
   struct depth_stencil_texture_init {
      DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
      uint16 width = 1;
      uint16 height = 1;
      bool shader_resource = false;

      D3D12_CLEAR_VALUE optimized_clear_value = {.Format = format,
                                                 .DepthStencil = {.Depth = 1.0f,
                                                                  .Stencil = 0x0}};

      operator texture_desc() const noexcept
      {
         return {.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                 .flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
                          (shader_resource ? D3D12_RESOURCE_FLAG_NONE
                                           : D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE),
                 .format = format,
                 .width = width,
                 .height = height,
                 .optimized_clear_value = optimized_clear_value};
      };
   };

   depth_stencil_texture(device& device, const depth_stencil_texture_init init_params,
                         const D3D12_RESOURCE_STATES initial_resource_state)
      : texture{device.create_texture(init_params, initial_resource_state)}
   {
      depth_stencil_view =
         device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
      device.device_d3d->CreateDepthStencilView(view_resource(), nullptr,
                                                depth_stencil_view[0].cpu);
   }

   using texture::alive;

   using texture::resource;

   using texture::array_size;
   using texture::depth;
   using texture::format;
   using texture::height;
   using texture::mip_levels;
   using texture::width;

   descriptor_allocation depth_stencil_view{};
};

}
