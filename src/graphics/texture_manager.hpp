#pragma once

#include "assets/asset_libraries.hpp"
#include "gpu/device.hpp"
#include "gpu/texture.hpp"

#include <concepts>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <DirectXTex.h>
#include <tbb/tbb.h>

namespace sk::graphics {

struct updated_texture {
   const std::string& name;
   const std::shared_ptr<gpu::texture>& texture;
};

class texture_manager {
public:
   texture_manager(gpu::device& gpu_device,
                   assets::library<assets::texture::texture>& texture_assets)
      : _texture_assets{texture_assets}, _gpu_device{&gpu_device}
   {
      _null_diffuse_map = std::make_shared<gpu::texture>(
         gpu_device.create_texture({.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                                    .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
                                   D3D12_RESOURCE_STATE_COMMON));

      _copy_fence_wait_value = [this] {
         assets::texture::texture cpu_null_diffuse{
            {.width = 1,
             .height = 1,
             .format = assets::texture::texture_format::r8g8b8a8_unorm_srgb}};

         cpu_null_diffuse.store({.mip_level = 0}, {0, 0},
                                float4{0.75f, 0.75f, 0.75f, 1.0f});

         return init_texture_async(*_null_diffuse_map, cpu_null_diffuse);
      }();
   };

   auto aquire_if(const std::string& name) -> std::shared_ptr<gpu::texture>
   {
      if (auto loaded = aquire_if_loaded(name); loaded) return loaded;

      auto cpu_texture = _texture_assets.aquire_if(lowercase_string{name});

      std::scoped_lock lock{_shared_mutex};

      if (not cpu_texture) return nullptr;

      enqueue_create_texture(name, cpu_texture);

      return nullptr;
   }

   auto copy_fence_wait_value() -> UINT64
   {
      std::shared_lock lock{_shared_mutex};

      return _copy_fence_wait_value;
   }

   auto null_diffuse_map() -> std::shared_ptr<gpu::texture>
   {
      return _null_diffuse_map;
   }

   template<std::invocable<updated_texture> Callback>
   void process_updated_textures(Callback&& callback)
   {
      std::scoped_lock lock{_shared_mutex};

      for (auto& texture : _copied_textures) {
         callback({.name = texture.first, .texture = texture.second});
      }

      _copied_textures.clear();

      for (auto [name, cpu_texture] : _texture_assets.loaded_assets()) {
         enqueue_create_texture(name, cpu_texture);
      }
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

   void enqueue_create_texture(const std::string& name,
                               std::shared_ptr<assets::texture::texture> cpu_texture)
   {
      if (auto [it, inserted] = _pending_textures.insert(name); not inserted) {
         return;
      }

      _creation_tasks.run([this, name = name, cpu_texture = std::move(cpu_texture)] {
         // ICE workaround...
         const gpu::texture_desc texture_desc = [&] {
            gpu::texture_desc texture_desc;

            texture_desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            texture_desc.format = cpu_texture->dxgi_format();
            texture_desc.width = cpu_texture->width();
            texture_desc.height = cpu_texture->height();
            texture_desc.mip_levels = cpu_texture->mip_levels();

            return texture_desc;
         }();

         auto texture = std::make_shared<gpu::texture>(
            _gpu_device->create_texture(texture_desc, D3D12_RESOURCE_STATE_COMMON));

         uint64 resource_copy_fence_value = init_texture_async(*texture, *cpu_texture);

         std::scoped_lock lock{_shared_mutex};

         _ready_textures.emplace(name, texture);
         _pending_textures.erase(name);
         _copied_textures.emplace_back(name, texture);
         _copy_fence_wait_value =
            std::max(_copy_fence_wait_value, resource_copy_fence_value);
      });
   }

   auto init_texture_async(gpu::texture& texture,
                           const assets::texture::texture& cpu_texture) -> UINT64
   {
      auto copy_context = _gpu_device->copy_manager.aquire_context();

      const gpu::buffer_desc upload_buffer_desc = [&] {
         gpu::buffer_desc upload_buffer_desc;

         upload_buffer_desc.size = static_cast<uint32>(cpu_texture.size());

         return upload_buffer_desc;
      }();

      ID3D12Resource& upload_buffer =
         copy_context.create_upload_resource(upload_buffer_desc);

      std::byte* const upload_buffer_ptr = [&] {
         const D3D12_RANGE read_range{};
         void* map_void_ptr = nullptr;

         throw_if_failed(upload_buffer.Map(0, &read_range, &map_void_ptr));

         return static_cast<std::byte*>(map_void_ptr);
      }();

      std::memcpy(upload_buffer_ptr, cpu_texture.data(), cpu_texture.size());

      const D3D12_RANGE write_range{0, cpu_texture.size()};
      upload_buffer.Unmap(0, &write_range);

      auto& command_list = copy_context.command_list;

      for (uint32 i = 0; i < cpu_texture.subresource_count(); ++i) {
         const auto& cpu_subresource = cpu_texture.subresource(i);

         const D3D12_TEXTURE_COPY_LOCATION dest_location{.pResource =
                                                            texture.resource(),
                                                         .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                                                         .SubresourceIndex = i};
         const D3D12_TEXTURE_COPY_LOCATION
            src_location{.pResource = &upload_buffer,
                         .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
                         .PlacedFootprint = {
                            .Offset = cpu_subresource.offset(),
                            .Footprint = {.Format = texture.format(),
                                          .Width = cpu_subresource.width(),
                                          .Height = cpu_subresource.height(),
                                          .Depth = 1,
                                          .RowPitch = cpu_subresource.row_pitch()},
                         }};

         command_list.CopyTextureRegion(&dest_location, 0, 0, 0, &src_location, nullptr);
      }

      return _gpu_device->copy_manager.close_and_execute(copy_context);
   }

   assets::library<assets::texture::texture>& _texture_assets;
   gsl::not_null<gpu::device*> _gpu_device; // Do NOT change while _creation_tasks has active tasks queued.

   std::shared_mutex _shared_mutex;
   std::unordered_map<std::string, std::shared_ptr<gpu::texture>> _ready_textures;
   std::unordered_set<std::string> _pending_textures;
   std::vector<std::pair<std::string, std::shared_ptr<gpu::texture>>> _copied_textures;

   uint64 _copy_fence_wait_value = 0;

   tbb::task_group _creation_tasks;

   std::shared_ptr<gpu::texture> _null_diffuse_map;
};

}