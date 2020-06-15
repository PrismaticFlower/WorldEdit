#pragma once

#include "gpu/device.hpp"
#include "gpu/texture.hpp"

#include <memory>

namespace sk::graphics {

class texture_manager {
public:
   texture_manager(gpu::device& gpu_device) : _gpu_device{&gpu_device}
   {
      _null_diffuse_map = std::make_shared<gpu::texture>(
         gpu_device.create_texture({.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                                    .format = DXGI_FORMAT_R8G8B8A8_TYPELESS},
                                   D3D12_RESOURCE_STATE_COMMON));
   };

   auto aquire_if(const std::string_view) -> std::shared_ptr<gpu::texture>
   {
      return nullptr;
   }

   auto null_diffuse_map() -> std::shared_ptr<gpu::texture>
   {
      return _null_diffuse_map;
   }

private:
   gsl::not_null<gpu::device*> _gpu_device; // Do NOT change while _creation_tasks has active tasks queued.

   std::shared_ptr<gpu::texture> _null_diffuse_map;
};

}