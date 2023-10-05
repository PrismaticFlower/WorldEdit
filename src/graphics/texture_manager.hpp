#pragma once

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "copy_command_list_pool.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "output_stream.hpp"

#include <concepts>
#include <memory>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

namespace we::graphics {

struct texture_manager;

struct world_texture_load_token {
   asset_ref<assets::texture::texture> asset_ref;
};

enum class world_texture_dimension { _2d, cube };

struct world_texture {
   world_texture(gpu::device& device, gpu::unique_resource_handle texture,
                 const DXGI_FORMAT format, const world_texture_dimension dimension);

   ~world_texture();

   world_texture(world_texture&&) = delete;
   auto operator=(world_texture&&) -> world_texture& = delete;

   gpu::resource_view srv;
   gpu::resource_view srv_srgb;
   gpu::resource_handle texture;
   world_texture_dimension dimension;

private:
   friend texture_manager;

   gpu::device& _device;
};

using updated_textures =
   absl::flat_hash_map<lowercase_string, std::shared_ptr<const world_texture>>;

enum class texture_status {
   ready,
   loading,
   errored,
   missing,
   dimension_mismatch
};

struct texture_manager {
   texture_manager(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
                   std::shared_ptr<async::thread_pool> thread_pool,
                   assets::library<assets::texture::texture>& texture_assets,
                   output_stream& error_output);

   /// @brief Gets the specified texture or returns a default texture if it is not available.
   /// @param name Name of the texture to get.
   /// @param expected_dimension The dimension of the texture to get. A texture will only be returned if this matches it else default_texture will be returned.
   /// @param default_texture Texture to return if the requested texture is not available.
   /// @return The texture or default_texture.
   auto at_or(const lowercase_string& name,
              const world_texture_dimension expected_dimension,
              std::shared_ptr<const world_texture> default_texture)
      -> std::shared_ptr<const world_texture>;

   /// @brief Acquire a shared_ptr to be used as a token representing interest in a texture. A texture will not be removed from the updated queue while it has load tokens outstanding.
   /// @param name Name of the texture to get.
   /// @return The interest token.
   auto acquire_load_token(const lowercase_string& name) noexcept
      -> std::shared_ptr<const world_texture_load_token>;

   /// @brief Texture with a color value of 0.75, 0.75, 0.75, 1.0.
   /// @return The texture.
   auto null_diffuse_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_diffuse_map;
   }

   /// @brief Texture with a color value of 0.5, 0.5, 1.0, 1.0.
   /// @return The texture.
   auto null_normal_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_normal_map;
   }

   /// @brief Texture with a color value of 0.5, 0.5, 0.5, 1.0.
   /// @return The texture.
   auto null_detail_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_detail_map;
   }

   /// @brief Cube texture with a color value of 0.0, 0.0, 0.0, 1.0.
   /// @return The texture.
   auto null_cube_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_cube_map;
   }

   /// @brief Texture with a color value of 1.0, 1.0, 1.0, 1.0.
   /// @return The texture.
   auto null_color_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_color_map;
   }

   /// @brief Allows processing updated textures through a callback.
   /// @param callback The callback to invoke with a reference to the updated textures. The reference is only safe to use until the callback returns.
   void eval_updated_textures(std::invocable<const updated_textures&> auto callback) noexcept
   {
      update_textures();

      std::scoped_lock lock{_mutex, _texture_load_tokens_mutex};

      callback(_copied_textures);

      absl::erase_if(_copied_textures,
                     [this](const std::pair<const lowercase_string,
                                            std::shared_ptr<const world_texture>>& copied) {
                        if (auto token = _texture_load_tokens.find(copied.first);
                            token != _texture_load_tokens.end()) {
                           return token->second.expired();
                        }

                        return true;
                     });

      absl::erase_if(_texture_load_tokens,
                     [this](const std::pair<const lowercase_string, std::weak_ptr<const world_texture_load_token>>&
                               token) { return token.second.expired(); });
   }

   /// @brief Call at the start of a frame to update textures that have been created asynchronously. eval_updated_textures() implicitly calls this.
   void update_textures() noexcept;

   /// @brief Query the status of a texture.
   /// @param name The name of the texture to query the status of.
   /// @param dimension The desired dimension of the texture to query the status.
   /// @return The status of the texture.
   auto status(const lowercase_string& name,
               const world_texture_dimension dimension) const noexcept -> texture_status;

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

   void texture_load_failure(const lowercase_string& name,
                             asset_ref<assets::texture::texture> asset) noexcept;

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

   struct pending_create_texture {
      async::task<std::shared_ptr<const world_texture>> task;
      asset_ref<assets::texture::texture> asset;
   };

   assets::library<assets::texture::texture>& _texture_assets;
   gpu::device& _device;
   copy_command_list_pool& _copy_command_list_pool;

   mutable std::shared_mutex _mutex;
   absl::flat_hash_map<lowercase_string, texture_state> _textures;
   absl::flat_hash_map<lowercase_string, asset_ref<assets::texture::texture>> _pending_loads;
   absl::flat_hash_map<lowercase_string, pending_create_texture> _pending_creations;
   absl::flat_hash_map<lowercase_string, std::shared_ptr<const world_texture>> _copied_textures;
   absl::flat_hash_set<lowercase_string> _failed_creations;

   std::shared_mutex _texture_load_tokens_mutex;
   absl::flat_hash_map<lowercase_string, std::weak_ptr<const world_texture_load_token>> _texture_load_tokens;

   std::shared_ptr<async::thread_pool> _thread_pool;

   std::shared_ptr<world_texture> _null_diffuse_map;
   std::shared_ptr<world_texture> _null_normal_map;
   std::shared_ptr<world_texture> _null_detail_map;
   std::shared_ptr<world_texture> _null_cube_map;
   std::shared_ptr<world_texture> _null_color_map;

   output_stream& _error_output;

   event_listener<void(const lowercase_string&, asset_ref<assets::texture::texture>,
                       asset_data<assets::texture::texture>)>
      _asset_load_listener = _texture_assets.listen_for_loads(
         [this](const lowercase_string& name, asset_ref<assets::texture::texture> asset,
                asset_data<assets::texture::texture> data) {
            texture_loaded(name, asset, data);
         });
   event_listener<void(const lowercase_string&, asset_ref<assets::texture::texture>)> _asset_load_failure_listener =
      _texture_assets.listen_for_load_failures(
         [this](const lowercase_string& name, asset_ref<assets::texture::texture> asset) {
            texture_load_failure(name, asset);
         });
};

}
