#pragma once

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "gpu/device.hpp"
#include "gpu/texture.hpp"

#include <concepts>
#include <memory>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>

namespace we::graphics {

struct world_texture {
   uint32 srv_index = 0;
   uint32 srv_srgb_index = 0;
   std::shared_ptr<const gpu::texture> texture;
};

using updated_textures =
   absl::flat_hash_map<lowercase_string, std::shared_ptr<const world_texture>>;

class texture_manager {
public:
   texture_manager(gpu::device& gpu_device,
                   std::shared_ptr<async::thread_pool> thread_pool,
                   assets::library<assets::texture::texture>& texture_assets)
      : _texture_assets{texture_assets}, _gpu_device{&gpu_device}, _thread_pool{thread_pool}
   {
      using assets::texture::texture_format;

      const auto null_texture_init = [&](const float4 v, const texture_format format) {
         assets::texture::texture cpu_null_texture{
            {.width = 1, .height = 1, .format = format}};

         cpu_null_texture.store({.mip_level = 0}, {0, 0}, v);

         const gpu::texture_desc texture_desc = [&] {
            gpu::texture_desc texture_desc;

            texture_desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            texture_desc.format = cpu_null_texture.dxgi_format();
            texture_desc.width = cpu_null_texture.width();
            texture_desc.height = cpu_null_texture.height();
            texture_desc.mip_levels = cpu_null_texture.mip_levels();

            return texture_desc;
         }();

         auto resources = std::make_shared<texture_resources>(
            _gpu_device->create_texture(texture_desc, D3D12_RESOURCE_STATE_COPY_DEST),
            _gpu_device->allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2));

         init_texture_async(resources->texture, cpu_null_texture);
         init_texture_descriptors(resources->texture, resources->descriptors);

         auto result = std::make_shared<world_texture>(
            world_texture{.srv_index = resources->descriptors.base_index(),
                          .srv_srgb_index = resources->descriptors.base_index() + 1,
                          .texture =
                             std::shared_ptr<const gpu::texture>{resources,
                                                                 &resources->texture}});
         return result;
      };

      _null_diffuse_map = null_texture_init(float4{0.75f, 0.75f, 0.75f, 1.0f},
                                            texture_format::r8g8b8a8_unorm_srgb);

      _null_normal_map = null_texture_init(float4{0.5f, 0.5f, 1.0f, 1.0f},
                                           texture_format::r8g8b8a8_unorm);
   };

   /// @brief Gets the specified texture or returns a default texture if it is not available.
   /// @param name Name of the texture to get.
   /// @param default_texture Texture to return if the requested texture is not available.
   /// @return The texture or default_texture.
   auto at_or(const lowercase_string& name,
              std::shared_ptr<const world_texture> default_texture)
      -> std::shared_ptr<const world_texture>
   {
      if (name.empty()) return default_texture;

      // Try to find an already existing texture using a shared lock.
      {
         std::shared_lock lock{_shared_mutex};

         if (auto state_entry = _textures.find(name); state_entry != _textures.end()) {
            const auto& [_, state] = *state_entry;

            if (auto texture = state.texture.lock(); texture) {
               return texture;
            }
         }
      }

      // Try and create a new texture.
      {
         std::scoped_lock lock{_shared_mutex};

         auto& state = _textures[name];

         if (auto texture = state.texture.lock(); texture) {
            // Not a mistake, the texture could've been created inbetween releasing the shared lock and retaking it exlcusively.

            return texture;
         }
         else {
            state.asset = _texture_assets[name];

            if (auto asset_data = state.asset.get_if(); asset_data) {
               if (not _pending_textures.contains(name)) {
                  create_texture_async(name, state.asset, asset_data);
               }
            }
         }
      }

      return default_texture;
   }

   /// @brief Texture with a color value of 0.75, 0.75, 0.75, 1.0.
   /// @return The texture.
   auto null_diffuse_map() -> std::shared_ptr<const world_texture>
   {
      return _null_diffuse_map;
   }

   /// @brief Texture with a color value of 0.5, 0.5, 1.0, 1.0.
   /// @return The texture.
   auto null_normal_map() -> std::shared_ptr<const world_texture>
   {
      return _null_normal_map;
   }

   /// @brief Allows processing updated textures through a callback.
   /// @param callback The callback to invoke with a reference to the updated textures. The reference is only safe to use until the callback returns.
   void eval_updated_textures(std::invocable<const updated_textures&> auto callback) noexcept
   {
      update_textures();

      std::scoped_lock lock{_shared_mutex};

      callback(_copied_textures);

      _copied_textures.clear();
   }

   /// @brief Call at the start of a frame to update textures that have been created asynchronously. eval_updated_textures() implicitly calls this.
   void update_textures() noexcept
   {
      std::scoped_lock lock{_shared_mutex};

      for (auto it = _pending_textures.begin(); it != _pending_textures.end();) {
         auto elem_it = it++;
         auto& [name, pending_create] = *elem_it;

         if (pending_create.task.ready()) {
            auto texture = pending_create.task.get();

            _textures.insert_or_assign(name,
                                       texture_state{.texture = texture,
                                                     .asset = pending_create.asset});
            _copied_textures.insert_or_assign(name, texture);

            _pending_textures.erase(elem_it);
         }
      }
   }

private:
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

      gpu::copy_command_list& command_list = copy_context.command_list();

      for (uint32 i = 0; i < cpu_texture.subresource_count(); ++i) {
         const auto& cpu_subresource = cpu_texture.subresource(i);

         const D3D12_TEXTURE_COPY_LOCATION dest_location{.pResource =
                                                            texture.view_resource(),
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

         command_list.copy_texture_region(dest_location, 0, 0, 0, src_location);
      }

      _gpu_device->copy_manager.close_and_execute(copy_context);
   }

   void init_texture_descriptors(gpu::texture& texture, gpu::descriptor_range descriptors)
   {
      _gpu_device->create_shader_resource_view(
         *texture.view_resource(),
         gpu::shader_resource_view_desc{.format = texture.format(),
                                        .type_description = gpu::texture2d_srv{}},
         descriptors[0]);

      _gpu_device->create_shader_resource_view(
         *texture.view_resource(),
         gpu::shader_resource_view_desc{.format = get_srgb_format(texture.format()),
                                        .type_description = gpu::texture2d_srv{}},
         descriptors[1]);
   }

   /// @brief Creates a texture asynchronously. _shared_mutex must be held before calling this.
   void create_texture_async(const lowercase_string& name,
                             asset_ref<assets::texture::texture> asset,
                             asset_data<assets::texture::texture> data)
   {
      pending_texture& pending = _pending_textures[name];

      if (pending.task.valid()) pending.task.cancel();

      pending =
         {.task = _thread_pool->exec(
             async::task_priority::low,
             [=]() -> std::shared_ptr<const world_texture> {
                const gpu::texture_desc texture_desc = [&] {
                   gpu::texture_desc texture_desc;

                   texture_desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                   texture_desc.format = data->dxgi_format();
                   texture_desc.width = data->width();
                   texture_desc.height = data->height();
                   texture_desc.mip_levels = data->mip_levels();

                   return texture_desc;
                }();

                auto resources = std::make_shared<texture_resources>(
                   _gpu_device->create_texture(texture_desc, D3D12_RESOURCE_STATE_COPY_DEST),
                   _gpu_device->allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                                     2));

                init_texture_async(resources->texture, *data);
                init_texture_descriptors(resources->texture, resources->descriptors);

                return std::make_shared<const world_texture>(world_texture{
                   .srv_index = resources->descriptors.base_index(),
                   .srv_srgb_index = resources->descriptors.base_index() + 1,
                   .texture = std::shared_ptr<const gpu::texture>{resources,
                                                                  &resources->texture}});
             }),
          .asset = asset};

      if (pending.asset.use_count() == 0) __debugbreak();
   }

   void texture_loaded(const lowercase_string& name,
                       asset_ref<assets::texture::texture> asset,
                       asset_data<assets::texture::texture> data) noexcept
   {
      std::scoped_lock lock{_shared_mutex};

      create_texture_async(name, asset, data);
   }

   static auto get_srgb_format(const DXGI_FORMAT format) noexcept -> DXGI_FORMAT
   {
      // clang-format off
      if (format == DXGI_FORMAT_R8G8B8A8_UNORM) return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      if (format == DXGI_FORMAT_BC1_UNORM) return DXGI_FORMAT_BC1_UNORM_SRGB;
      if (format == DXGI_FORMAT_BC2_UNORM) return DXGI_FORMAT_BC2_UNORM_SRGB;
      if (format == DXGI_FORMAT_BC3_UNORM) return DXGI_FORMAT_BC3_UNORM_SRGB;
      if (format == DXGI_FORMAT_B8G8R8A8_UNORM) return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
      if (format == DXGI_FORMAT_B8G8R8X8_UNORM) return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
      if (format == DXGI_FORMAT_BC7_UNORM) return DXGI_FORMAT_BC7_UNORM_SRGB;
      // clang-format on

      return format;
   }

   struct texture_resources {
      gpu::texture texture;
      gpu::descriptor_allocation descriptors;
   };

   struct texture_state {
      std::weak_ptr<const world_texture> texture;
      asset_ref<assets::texture::texture> asset;
   };

   struct pending_texture {
      async::task<std::shared_ptr<const world_texture>> task;
      asset_ref<assets::texture::texture> asset;
   };

   assets::library<assets::texture::texture>& _texture_assets;
   gsl::not_null<gpu::device*> _gpu_device; // Do NOT change while _creation_tasks has active tasks queued.

   std::shared_mutex _shared_mutex;
   absl::flat_hash_map<lowercase_string, texture_state> _textures;
   absl::flat_hash_map<lowercase_string, pending_texture> _pending_textures;
   absl::flat_hash_map<lowercase_string, std::shared_ptr<const world_texture>> _copied_textures;

   std::shared_ptr<async::thread_pool> _thread_pool;

   std::shared_ptr<world_texture> _null_diffuse_map;
   std::shared_ptr<world_texture> _null_normal_map;

   event_listener<void(const lowercase_string&, asset_ref<assets::texture::texture>,
                       asset_data<assets::texture::texture>)>
      _asset_load_listener = _texture_assets.listen_for_loads(
         [this](const lowercase_string& name, asset_ref<assets::texture::texture> asset,
                asset_data<assets::texture::texture> data) {
            texture_loaded(name, asset, data);
         });
};

}
