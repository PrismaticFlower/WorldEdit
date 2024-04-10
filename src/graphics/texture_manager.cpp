
#include "texture_manager.hpp"
#include "assets/texture/texture.hpp"
#include "gpu/resource.hpp"
#include "utility/string_ops.hpp"

#include <memory>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

namespace we::graphics {

namespace {

auto get_srgb_format(const DXGI_FORMAT format) noexcept -> DXGI_FORMAT
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

auto strip_srgb_format(const DXGI_FORMAT format) noexcept -> DXGI_FORMAT
{
   // clang-format off
      if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) return DXGI_FORMAT_R8G8B8A8_UNORM;
      if (format == DXGI_FORMAT_BC1_UNORM_SRGB) return DXGI_FORMAT_BC1_UNORM;
      if (format == DXGI_FORMAT_BC2_UNORM_SRGB) return DXGI_FORMAT_BC2_UNORM;
      if (format == DXGI_FORMAT_BC3_UNORM_SRGB) return DXGI_FORMAT_BC3_UNORM;
      if (format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) return DXGI_FORMAT_B8G8R8A8_UNORM;
      if (format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB) return DXGI_FORMAT_B8G8R8X8_UNORM;
      if (format == DXGI_FORMAT_BC7_UNORM_SRGB) return DXGI_FORMAT_BC7_UNORM;
   // clang-format on

   return format;
}

using updated_textures_map =
   absl::flat_hash_map<lowercase_string, std::shared_ptr<const world_texture>>;

}

struct texture_manager::impl {
   impl(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
        std::shared_ptr<async::thread_pool> thread_pool,
        assets::library<assets::texture::texture>& texture_assets,
        output_stream& error_output)
      : _texture_assets{texture_assets},
        _device{device},
        _copy_command_list_pool{copy_command_list_pool},
        _thread_pool{thread_pool},
        _error_output{error_output}
   {
      using assets::texture::texture_format;

      const auto null_texture_init = [&](const uint32 v, const texture_format format) {
         assets::texture::texture cpu_null_texture{
            {.width = 1, .height = 1, .format = format}};

         std::memcpy(cpu_null_texture.data(), &v, sizeof(v));

         gpu::unique_resource_handle texture =
            {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                     .format = cpu_null_texture.dxgi_format(),
                                     .width = cpu_null_texture.width(),
                                     .height = cpu_null_texture.height(),
                                     .mip_levels = cpu_null_texture.mip_levels()},
                                    gpu::barrier_layout::common,
                                    gpu::legacy_resource_state::common),
             _device.direct_queue};

         init_texture(texture.get(), cpu_null_texture);

         auto result =
            std::make_shared<world_texture>(_device, std::move(texture),
                                            cpu_null_texture.dxgi_format(),
                                            world_texture_dimension::_2d);

         return result;
      };

      _null_diffuse_map =
         null_texture_init(0xff'e2'e2'e2u, texture_format::r8g8b8a8_unorm_srgb);

      _null_normal_map =
         null_texture_init(0xff'ff'80'80u, texture_format::r8g8b8a8_unorm);

      _null_detail_map =
         null_texture_init(0xff'80'80'80u, texture_format::r8g8b8a8_unorm);

      _null_color_map =
         null_texture_init(0xff'ff'ff'ffu, texture_format::r8g8b8a8_unorm_srgb);

      const auto null_cube_texture_init = [&](const uint32 v,
                                              const texture_format format) {
         assets::texture::texture cpu_null_texture{{.width = 1,
                                                    .height = 1,
                                                    .array_size = 6,
                                                    .format = format,
                                                    .flags = {.cube_map = true}}};

         for (uint32 i = 0; i < 6; ++i) {
            std::memcpy(cpu_null_texture.subresource(i).data(), &v, sizeof(v));
         }

         gpu::unique_resource_handle texture =
            {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                     .format = cpu_null_texture.dxgi_format(),
                                     .width = cpu_null_texture.width(),
                                     .height = cpu_null_texture.height(),
                                     .mip_levels = cpu_null_texture.mip_levels(),
                                     .array_size = cpu_null_texture.array_size()},
                                    gpu::barrier_layout::common,
                                    gpu::legacy_resource_state::common),
             _device.direct_queue};

         init_texture(texture.get(), cpu_null_texture);

         auto result =
            std::make_shared<world_texture>(_device, std::move(texture),
                                            cpu_null_texture.dxgi_format(),
                                            world_texture_dimension::cube);

         return result;
      };

      _null_cube_map =
         null_cube_texture_init(0xff'00'00'00u, texture_format::r8g8b8a8_unorm_srgb);
   }

   auto at_or(const lowercase_string& name,
              const world_texture_dimension expected_dimension,
              std::shared_ptr<const world_texture> default_texture)
      -> std::shared_ptr<const world_texture>
   {
      if (name.empty()) return default_texture;

      // Try to find an already existing texture using a shared lock.
      {
         std::shared_lock lock{_mutex};

         if (auto state_entry = _textures.find(name); state_entry != _textures.end()) {
            const auto& [_, state] = *state_entry;

            if (auto texture = state.texture.lock(); texture) {
               if (texture->dimension == expected_dimension) return texture;
            }
         }

         if (_pending_loads.contains(name) or _pending_creations.contains(name) or
             _failed_creations.contains(name)) {
            return default_texture;
         }
      }

      asset_ref asset = _texture_assets[name];

      if (not asset.exists()) return default_texture;

      // Try and create a new texture.
      {
         std::scoped_lock lock{_mutex};

         // Not a mistake, the texture could've been created inbetween releasing the shared lock and retaking it exlcusively.
         if (auto state_entry = _textures.find(name); state_entry != _textures.end()) {
            const auto& [_, state] = *state_entry;

            if (auto texture = state.texture.lock(); texture) {
               if (texture->dimension == expected_dimension) return texture;
            }
         }

         const auto& [_, inserted] = _pending_loads.insert_or_assign(name, asset);

         if (not inserted or _pending_creations.contains(name)) {
            return default_texture;
         }

         if (auto asset_data = asset.get_if(); asset_data) {
            create_texture_async(name, asset, asset_data);
         }
      }

      return default_texture;
   }

   auto acquire_load_token(const lowercase_string& name) noexcept
      -> std::shared_ptr<const world_texture_load_token>
   {
      std::scoped_lock lock{_texture_load_tokens_mutex};

      if (auto existing = _texture_load_tokens.find(name);
          existing != _texture_load_tokens.end()) {
         auto token = existing->second.lock();

         if (token) return token;
      }

      auto token =
         std::make_shared<const world_texture_load_token>(_texture_assets[name]);

      _texture_load_tokens.insert_or_assign(name, token);

      return token;
   }

   auto null_diffuse_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_diffuse_map;
   }

   auto null_normal_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_normal_map;
   }

   auto null_detail_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_detail_map;
   }

   auto null_cube_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_cube_map;
   }

   auto null_color_map() const noexcept -> std::shared_ptr<const world_texture>
   {
      return _null_color_map;
   }

   void eval_updated_textures(function_ptr<void(const updated_textures&) noexcept> callback) noexcept
   {
      update_textures();

      std::scoped_lock lock{_mutex, _texture_load_tokens_mutex};

      static_assert(std::is_same_v<decltype(_copied_textures), updated_textures_map>, "'updated_textures::check expects' '_copied textures' to have the type 'updated_textures_map'");

      updated_textures updated_textures{&_copied_textures};

      callback(updated_textures);

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

   void update_textures() noexcept
   {
      std::scoped_lock lock{_mutex};

      for (auto it = _pending_creations.begin(); it != _pending_creations.end();) {
         auto elem_it = it++;
         auto& [name, pending_create] = *elem_it;

         if (pending_create.task.ready()) {
            std::shared_ptr<const world_texture> texture;

            try {
               texture = pending_create.task.get();
            }
            catch (std::runtime_error& e) {
               _failed_creations.emplace(name);

               _error_output
                  .write("Failed to create texture for GPU:\n   Name: "
                         "{}\n   Message.\n \n{}\n",
                         std::string_view{name}, string::indent(2, e.what()));
            }

            if (texture) {
               _textures.insert_or_assign(name,
                                          texture_state{.texture = texture,
                                                        .asset = pending_create.asset});
               _copied_textures.insert_or_assign(name, texture);
            }

            _pending_creations.erase(elem_it);
         }
      }
   }

   auto status(const lowercase_string& name,
               const world_texture_dimension dimension) const noexcept -> texture_status
   {
      std::shared_lock lock{_mutex};

      if (auto it = _textures.find(name); it != _textures.end()) {
         const texture_state& state = it->second;

         if (auto texture = state.texture.lock(); texture) {
            if (texture->dimension != dimension) {
               return texture_status::dimension_mismatch;
            }

            return texture_status::ready;
         }
      }

      if (_pending_loads.contains(name)) return texture_status::loading;
      if (_pending_creations.contains(name)) return texture_status::loading;
      if (_failed_creations.contains(name)) return texture_status::errored;

      return texture_status::missing;
   }

private:
   void init_texture(gpu::resource_handle texture,
                     const assets::texture::texture& cpu_texture)
   {
      pooled_copy_command_list command_list =
         _copy_command_list_pool.aquire_and_reset();

      gpu::unique_resource_handle upload_buffer{
         _device.create_buffer({.size = cpu_texture.size(),
                                .debug_name = "Texture Manager Upload Buffer"},
                               gpu::heap_type::upload),
         _device.background_copy_queue};

      std::byte* const upload_buffer_ptr =
         static_cast<std::byte*>(_device.map(upload_buffer.get(), 0, {}));

      std::memcpy(upload_buffer_ptr, cpu_texture.data(), cpu_texture.size());

      _device.unmap(upload_buffer.get(), 0, {0, cpu_texture.size()});

      for (uint32 i = 0; i < cpu_texture.subresource_count(); ++i) {
         const auto& cpu_subresource = cpu_texture.subresource(i);

         command_list->copy_buffer_to_texture(texture, i, 0, 0, 0,
                                              upload_buffer.get(),
                                              {.offset = cpu_subresource.offset(),
                                               .format = cpu_subresource.dxgi_format(),
                                               .width = cpu_subresource.width(),
                                               .height = cpu_subresource.height(),
                                               .depth = 1,
                                               .row_pitch =
                                                  cpu_subresource.row_pitch()});
      }

      command_list->close();

      _device.background_copy_queue.execute_command_lists(command_list.get());
   }

   /// @brief Creates a texture asynchronously. _shared_mutex must be held before calling this.
   void create_texture_async(const lowercase_string& name,
                             asset_ref<assets::texture::texture> asset,
                             asset_data<assets::texture::texture> data)
   {
      _failed_creations.erase(name);
      _pending_loads.erase(name);
      _pending_creations[name] =
         {.task = _thread_pool->exec(
             async::task_priority::low,
             [=]() -> std::shared_ptr<const world_texture> {
                gpu::unique_resource_handle texture =
                   {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                            .format = data->dxgi_format(),
                                            .width = data->width(),
                                            .height = data->height(),
                                            .mip_levels = data->mip_levels(),
                                            .array_size = data->array_size()},
                                           gpu::barrier_layout::common,
                                           gpu::legacy_resource_state::common),
                    _device.direct_queue};

                init_texture(texture.get(), *data);

                _device.background_copy_queue.wait_for_idle();

                return std::make_shared<world_texture>(_device, std::move(texture),
                                                       data->dxgi_format(),
                                                       data->flags().cube_map
                                                          ? world_texture_dimension::cube
                                                          : world_texture_dimension::_2d);
             }),
          .asset = asset};
   }

   void texture_loaded(const lowercase_string& name,
                       asset_ref<assets::texture::texture> asset,
                       asset_data<assets::texture::texture> data) noexcept
   {
      const bool load_token = [&] {
         std::shared_lock lock{_texture_load_tokens_mutex};

         if (auto it = _texture_load_tokens.find(name);
             it != _texture_load_tokens.end()) {
            return not it->second.expired();
         }

         return false;
      }();

      std::scoped_lock lock{_mutex};

      if (not _pending_loads.contains(name) and not _textures.contains(name) and
          not load_token) {
         return;
      }

      create_texture_async(name, asset, data);
   }

   void texture_load_failure(const lowercase_string& name,
                             asset_ref<assets::texture::texture> asset) noexcept
   {
      std::scoped_lock lock{_mutex};

      _pending_loads.erase(name);
      _failed_creations.emplace(name);
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

world_texture::world_texture(gpu::device& device, gpu::unique_resource_handle texture,
                             const DXGI_FORMAT format,
                             const world_texture_dimension dimension)
   : _device{device}, dimension{dimension}
{
   const bool texture_cube_view = dimension == world_texture_dimension::cube;

   srv = device.create_shader_resource_view(texture.get(),
                                            {.format = strip_srgb_format(format),
                                             .texture_cube_view = texture_cube_view});
   srv_srgb =
      device.create_shader_resource_view(texture.get(),
                                         {.format = get_srgb_format(format),
                                          .texture_cube_view = texture_cube_view});

   this->texture = texture.release();
}

world_texture::~world_texture()
{
   _device.direct_queue.release_resource_view(srv);
   _device.direct_queue.release_resource_view(srv_srgb);
   _device.direct_queue.release_resource(texture);
}

updated_textures::updated_textures(const void* copied_textures) noexcept
   : _copied_textures{copied_textures}
{
}

auto updated_textures::check(const lowercase_string& name) const noexcept
   -> std::shared_ptr<const world_texture>
{
   const updated_textures_map& copied_textures =
      *static_cast<const updated_textures_map*>(_copied_textures);

   if (auto it = copied_textures.find(name); it != copied_textures.end()) {
      return it->second;
   }

   return nullptr;
}

void updated_textures::eval_all(
   function_ptr<void(const lowercase_string&, const std::shared_ptr<const world_texture>&) noexcept>
      callback) const noexcept
{
   for (const auto& [name, texture] :
        *static_cast<const updated_textures_map*>(_copied_textures)) {
      callback(name, texture);
   }
}

texture_manager::texture_manager(gpu::device& device,
                                 copy_command_list_pool& copy_command_list_pool,
                                 std::shared_ptr<async::thread_pool> thread_pool,
                                 assets::library<assets::texture::texture>& texture_assets,
                                 output_stream& error_output)
   : _impl{device, copy_command_list_pool, std::move(thread_pool),
           texture_assets, error_output}
{
}

texture_manager::~texture_manager() = default;

auto texture_manager::at_or(const lowercase_string& name,
                            const world_texture_dimension expected_dimension,
                            std::shared_ptr<const world_texture> default_texture)
   -> std::shared_ptr<const world_texture>
{
   return _impl->at_or(name, expected_dimension, std::move(default_texture));
}

auto texture_manager::acquire_load_token(const lowercase_string& name) noexcept
   -> std::shared_ptr<const world_texture_load_token>
{
   return _impl->acquire_load_token(name);
}

auto texture_manager::null_diffuse_map() const noexcept
   -> std::shared_ptr<const world_texture>
{
   return _impl->null_diffuse_map();
}

auto texture_manager::null_normal_map() const noexcept
   -> std::shared_ptr<const world_texture>
{
   return _impl->null_normal_map();
}

auto texture_manager::null_detail_map() const noexcept
   -> std::shared_ptr<const world_texture>
{
   return _impl->null_detail_map();
}

auto texture_manager::null_cube_map() const noexcept
   -> std::shared_ptr<const world_texture>
{
   return _impl->null_cube_map();
}

auto texture_manager::null_color_map() const noexcept
   -> std::shared_ptr<const world_texture>
{
   return _impl->null_color_map();
}

void texture_manager::eval_updated_textures(
   function_ptr<void(const updated_textures&) noexcept> callback) noexcept
{
   return _impl->eval_updated_textures(callback);
}

void texture_manager::update_textures() noexcept
{
   return _impl->update_textures();
}

auto texture_manager::status(const lowercase_string& name,
                             const world_texture_dimension dimension) const noexcept
   -> texture_status
{
   return _impl->status(name, dimension);
}

}