#pragma once

#include "assets/asset_libraries.hpp"
#include "gpu/device.hpp"
#include "gpu/texture.hpp"

#include <concepts>
#include <memory>
#include <shared_mutex>
#include <vector>

#include <DirectXTex.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <tbb/tbb.h>

namespace sk::graphics {

struct updated_texture {
   const lowercase_string& name;
   std::shared_ptr<const gpu::texture> texture;
};

class texture_manager {
public:
   texture_manager(gpu::device& gpu_device,
                   assets::library<assets::texture::texture>& texture_assets)
      : _texture_assets{texture_assets}, _gpu_device{&gpu_device}
   {
      using assets::texture::texture_format;

      const auto null_texture_init = [&](const float4 v, const texture_format format) {
         assets::texture::texture cpu_null_texture{
            {.width = 1, .height = 1, .format = format}};

         cpu_null_texture.store({.mip_level = 0}, {0, 0}, v);

         auto result = std::make_shared<gpu::texture>(
            gpu_device.create_texture({.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                                       .format = cpu_null_texture.dxgi_format()},
                                      D3D12_RESOURCE_STATE_COMMON));

         init_texture_async(*result, cpu_null_texture);

         return result;
      };

      _null_diffuse_map = null_texture_init(float4{0.75f, 0.75f, 0.75f, 1.0f},
                                            texture_format::r8g8b8a8_unorm_srgb);

      _null_normal_map = null_texture_init(float4{0.5f, 0.5f, 1.0f, 1.0f},
                                           texture_format::r8g8b8a8_unorm_srgb);
   };

   /// @brief Gets the specified texture or returns a default texture if it is not available.
   /// @param name Name of the texture to get.
   /// @param default_texture Texture to return if the requested texture is not available.
   /// @return The texture or default_texture.
   auto at_or(const lowercase_string& name,
              std::shared_ptr<const gpu::texture> default_texture)
      -> std::shared_ptr<const gpu::texture>
   {
      if (name.empty()) return default_texture;

      {
         std::shared_lock lock{_shared_mutex};

         if (auto it = _textures.find(name); it != _textures.end()) {
            return it->second.texture;
         }
      }

      enqueue_create_texture(name);

      return default_texture;
   }

   /// @brief Texture with a color value of 0.75, 0.75, 0.75, 1.0.
   /// @return The texture.
   auto null_diffuse_map() -> std::shared_ptr<gpu::texture>
   {
      return _null_diffuse_map;
   }

   /// @brief Texture with a color value of 0.5, 0.5, 1.0, 1.0.
   /// @return The texture.
   auto null_normal_map() -> std::shared_ptr<gpu::texture>
   {
      return _null_normal_map;
   }

   /// @brief Process textures that have been updated since the last frame.
   /// @param callback A function to call with a updated_texture on the updated textures.
   void process_updated_textures(std::invocable<updated_texture> auto&& callback)
   {
      std::scoped_lock lock{_shared_mutex};

      for (auto& texture : _copied_textures) {
         std::invoke(callback, updated_texture{.name = texture.first,
                                               .texture = texture.second});
      }

      _copied_textures.clear();
   }

private:
   void enqueue_create_texture(const lowercase_string& name)
   {
      auto texture_asset = _texture_assets[name];
      auto texture_data = _texture_assets[name].get_if();

      if (auto [it, inserted] = _pending_textures.emplace(name, texture_asset);
          not inserted) {
         return;
      }

      if (not texture_data) return;

      _creation_tasks.run([this, texture_asset, texture_data, name = name] {
         texture_loaded(name, texture_asset, texture_data);
      });
   }

   void init_texture_async(gpu::texture& texture, const assets::texture::texture& cpu_texture)
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

      _gpu_device->copy_manager.close_and_execute(copy_context);
   }

   void texture_loaded(const lowercase_string& name,
                       asset_ref<assets::texture::texture> asset,
                       asset_data<assets::texture::texture> data) noexcept
   {
      const gpu::texture_desc texture_desc = [&] {
         gpu::texture_desc texture_desc;

         texture_desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
         texture_desc.format = data->dxgi_format();
         texture_desc.width = data->width();
         texture_desc.height = data->height();
         texture_desc.mip_levels = data->mip_levels();

         return texture_desc;
      }();

      auto texture = std::make_shared<gpu::texture>(
         _gpu_device->create_texture(texture_desc, D3D12_RESOURCE_STATE_COMMON));

      init_texture_async(*texture, *data);

      std::scoped_lock lock{_shared_mutex};

      _textures[name] = texture_state{.texture = texture, .asset = asset};
      _pending_textures.erase(name);
      _copied_textures.emplace_back(name, texture);
   }

   struct texture_state {
      std::shared_ptr<gpu::texture> texture;
      asset_ref<assets::texture::texture> asset;
   };

   assets::library<assets::texture::texture>& _texture_assets;
   gsl::not_null<gpu::device*> _gpu_device; // Do NOT change while _creation_tasks has active tasks queued.

   std::shared_mutex _shared_mutex;
   absl::flat_hash_map<lowercase_string, texture_state> _textures;
   absl::flat_hash_map<lowercase_string, asset_ref<assets::texture::texture>> _pending_textures;
   std::vector<std::pair<lowercase_string, std::shared_ptr<gpu::texture>>> _copied_textures;

   tbb::task_group _creation_tasks;

   std::shared_ptr<gpu::texture> _null_diffuse_map;
   std::shared_ptr<gpu::texture> _null_normal_map;

   event_listener<void(const lowercase_string&, asset_ref<assets::texture::texture>,
                       asset_data<assets::texture::texture>)>
      _asset_load_listener = _texture_assets.listen_for_loads(
         [this](const lowercase_string& name, asset_ref<assets::texture::texture> asset,
                asset_data<assets::texture::texture> data) {
            texture_loaded(name, asset, data);
         });
};

}
