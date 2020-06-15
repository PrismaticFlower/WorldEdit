#pragma once

#include "assets/asset_libraries.hpp"
#include "gpu/device.hpp"
#include "gpu/texture.hpp"

#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace sk::graphics {

class texture_manager {
public:
   texture_manager(gpu::device& gpu_device,
                   assets::library<assets::texture::texture>& texture_assets)
      : _texture_assets{texture_assets}, _gpu_device{&gpu_device}
   {
      _null_diffuse_map = std::make_shared<gpu::texture>(
         gpu_device.create_texture({.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                                    .format = DXGI_FORMAT_R8G8B8A8_TYPELESS},
                                   D3D12_RESOURCE_STATE_COMMON));
   };

   auto aquire_if(const std::string& name) -> std::shared_ptr<gpu::texture>
   {
      if (auto loaded = aquire_if_loaded(name); loaded) return loaded;

      return nullptr;
   }

   auto null_diffuse_map() -> std::shared_ptr<gpu::texture>
   {
      return _null_diffuse_map;
   }

private:
   auto aquire_if_loaded(const std::string& name) -> std::shared_ptr<gpu::texture>
   {
      std::shared_lock lock{_shared_mutex};

      if (auto it = _ready_textures.find(name); it != _ready_textures.end()) {
         return it->second;
      }

      return nullptr;
   }

   assets::library<assets::texture::texture>& _texture_assets;
   gsl::not_null<gpu::device*> _gpu_device; // Do NOT change while _creation_tasks has active tasks queued.

   std::shared_mutex _shared_mutex;
   std::unordered_map<std::string, std::shared_ptr<gpu::texture>> _ready_textures;
   std::unordered_set<std::string> _pending_textures;

   std::shared_ptr<gpu::texture> _null_diffuse_map;
};

}