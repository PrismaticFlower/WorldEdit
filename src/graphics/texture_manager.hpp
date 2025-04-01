#pragma once

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "copy_command_list_pool.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "output_stream.hpp"
#include "utility/function_ptr.hpp"
#include "utility/implementation_storage.hpp"

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

struct updated_textures {
   auto check(const lowercase_string& name) const noexcept
      -> std::shared_ptr<const world_texture>;

   void eval_all(
      function_ptr<void(const lowercase_string&, const std::shared_ptr<const world_texture>&) noexcept>
         callback) const noexcept;

private:
   friend texture_manager;

   updated_textures(const void* copied_textures) noexcept;

   ~updated_textures() = default;

   const void* _copied_textures = nullptr;
};

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

   texture_manager(const texture_manager&) = delete;
   texture_manager(texture_manager&&) = delete;

   ~texture_manager();

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
   auto null_diffuse_map() const noexcept
      -> const std::shared_ptr<const world_texture>&;

   /// @brief Texture with a color value of 0.5, 0.5, 1.0, 1.0.
   /// @return The texture.
   auto null_normal_map() const noexcept
      -> const std::shared_ptr<const world_texture>&;

   /// @brief Texture with a color value of 0.5, 0.5, 0.5, 1.0.
   /// @return The texture.
   auto null_detail_map() const noexcept
      -> const std::shared_ptr<const world_texture>&;

   /// @brief Cube texture with a color value of 0.0, 0.0, 0.0, 1.0.
   /// @return The texture.
   auto null_cube_map() const noexcept -> const std::shared_ptr<const world_texture>&;

   /// @brief Texture with a color value of 1.0, 1.0, 1.0, 1.0.
   /// @return The texture.
   auto null_color_map() const noexcept
      -> const std::shared_ptr<const world_texture>&;

   /// @brief Allows processing updated textures through a callback.
   /// @param callback The callback to invoke with a reference to the updated textures. The reference is only safe to use until the callback returns.
   void eval_updated_textures(function_ptr<void(const updated_textures&) noexcept> callback) noexcept;

   /// @brief Call at the start of a frame to update textures that have been created asynchronously. eval_updated_textures() implicitly calls this.
   void update_textures() noexcept;

   /// @brief Query the status of a texture.
   /// @param name The name of the texture to query the status of.
   /// @param dimension The desired dimension of the texture to query the status.
   /// @return The status of the texture.
   auto status(const lowercase_string& name,
               const world_texture_dimension dimension) const noexcept -> texture_status;

private:
   struct impl;

   implementation_storage<impl, 432> _impl;
};

}
