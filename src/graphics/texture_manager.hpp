#pragma once

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "copy_command_list_pool.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"

#include <concepts>
#include <memory>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>

namespace we::graphics {

struct texture_manager;

struct world_texture {
   world_texture(gpu::device& device, gpu::unique_resource_handle texture,
                 const DXGI_FORMAT format);

   ~world_texture();

   world_texture(world_texture&&) = delete;
   auto operator=(world_texture&&) -> world_texture& = delete;

   gpu::resource_view srv;
   gpu::resource_view srv_srgb;
   gpu::resource_handle texture;

private:
   friend texture_manager;

   gpu::device& _device;
};

using updated_textures =
   absl::flat_hash_map<lowercase_string, std::shared_ptr<const world_texture>>;

struct texture_manager {
   texture_manager(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
                   std::shared_ptr<async::thread_pool> thread_pool,
                   assets::library<assets::texture::texture>& texture_assets);

   /// @brief Gets the specified texture or returns a default texture if it is not available.
   /// @param name Name of the texture to get.
   /// @param default_texture Texture to return if the requested texture is not available.
   /// @return The texture or default_texture.
   auto at_or(const lowercase_string& name,
              std::shared_ptr<const world_texture> default_texture)
      -> std::shared_ptr<const world_texture>;

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
   void update_textures() noexcept;

private:
   void init_texture(gpu::resource_handle texture,
                     const assets::texture::texture& cpu_texture);

   /// @brief Creates a texture asynchronously. _shared_mutex must be held before calling this.
   void create_texture_async(const lowercase_string& name,
                             asset_ref<assets::texture::texture> asset,
                             asset_data<assets::texture::texture> data);

   void texture_loaded(const lowercase_string& name,
                       asset_ref<assets::texture::texture> asset,
                       asset_data<assets::texture::texture> data) noexcept;

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

   struct texture_state {
      std::weak_ptr<const world_texture> texture;
      asset_ref<assets::texture::texture> asset;
   };

   struct pending_texture {
      async::task<std::shared_ptr<const world_texture>> task;
      asset_ref<assets::texture::texture> asset;
   };

   assets::library<assets::texture::texture>& _texture_assets;
   gpu::device& _device;
   copy_command_list_pool& _copy_command_list_pool;

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
