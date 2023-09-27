
#include "texture_manager.hpp"
#include "assets/texture/texture.hpp"

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

}

world_texture::world_texture(gpu::device& device, gpu::unique_resource_handle texture,
                             const DXGI_FORMAT format,
                             const world_texture_dimension dimension)
   : _device{device}, dimension{dimension}
{
   const bool texture_cube_view = dimension == world_texture_dimension::cube;

   srv = device.create_shader_resource_view(
      texture.get(), {.format = strip_srgb_format(format),
                      .texture2d = {.texture_cube_view = texture_cube_view},
                      .texture2d_array = {.texture_cube_view = texture_cube_view}});
   srv_srgb = device.create_shader_resource_view(
      texture.get(), {.format = get_srgb_format(format),
                      .texture2d = {.texture_cube_view = texture_cube_view},
                      .texture2d_array = {.texture_cube_view = texture_cube_view}});

   this->texture = texture.release();
}

world_texture::~world_texture()
{
   _device.direct_queue.release_resource_view(srv);
   _device.direct_queue.release_resource_view(srv_srgb);
   _device.direct_queue.release_resource(texture);
}

texture_manager::texture_manager(gpu::device& device,
                                 copy_command_list_pool& copy_command_list_pool,
                                 std::shared_ptr<async::thread_pool> thread_pool,
                                 assets::library<assets::texture::texture>& texture_assets)
   : _texture_assets{texture_assets},
     _device{device},
     _copy_command_list_pool{copy_command_list_pool},
     _thread_pool{thread_pool}
{
   using assets::texture::texture_format;

   const auto null_texture_init = [&](const float4 v, const texture_format format) {
      assets::texture::texture cpu_null_texture{
         {.width = 1, .height = 1, .format = format}};

      cpu_null_texture.store({.mip_level = 0}, {0, 0}, v);

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

      auto result = std::make_shared<world_texture>(_device, std::move(texture),
                                                    cpu_null_texture.dxgi_format(),
                                                    world_texture_dimension::_2d);

      return result;
   };

   _null_diffuse_map = null_texture_init(float4{0.75f, 0.75f, 0.75f, 1.0f},
                                         texture_format::r8g8b8a8_unorm_srgb);

   _null_normal_map = null_texture_init(float4{0.5f, 0.5f, 1.0f, 1.0f},
                                        texture_format::r8g8b8a8_unorm);

   _null_detail_map = null_texture_init(float4{0.5f, 0.5f, 0.5f, 1.0f},
                                        texture_format::r8g8b8a8_unorm);

   _null_color_map = null_texture_init(float4{1.0f, 1.0f, 1.0f, 1.0f},
                                       texture_format::r8g8b8a8_unorm_srgb);

   const auto null_cube_texture_init = [&](const float4 v, const texture_format format) {
      assets::texture::texture cpu_null_texture{{.width = 1,
                                                 .height = 1,
                                                 .array_size = 6,
                                                 .format = format,
                                                 .flags = {.cube_map = true}}};

      for (uint32 i = 0; i < 6; ++i) {
         cpu_null_texture.store({.array_index = i}, {0, 0}, v);
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

      auto result = std::make_shared<world_texture>(_device, std::move(texture),
                                                    cpu_null_texture.dxgi_format(),
                                                    world_texture_dimension::cube);

      return result;
   };

   _null_cube_map = null_cube_texture_init({0.0f, 0.0f, 0.0f, 1.0f},
                                           texture_format::r8g8b8a8_unorm_srgb);
};

auto texture_manager::at_or(const lowercase_string& name,
                            const world_texture_dimension expected_dimension,
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
            if (texture->dimension == expected_dimension) return texture;
         }
      }
   }

   // Try and create a new texture.
   {
      std::scoped_lock lock{_shared_mutex};

      auto& state = _textures[name];

      if (auto texture = state.texture.lock(); texture) {
         // Not a mistake, the texture could've been created inbetween releasing the shared lock and retaking it exlcusively.

         if (texture->dimension == expected_dimension) return texture;
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

auto texture_manager::acquire_load_token(const lowercase_string& name) noexcept
   -> std::shared_ptr<const world_texture_load_token>
{
   std::scoped_lock lock{_texture_load_tokens_mutex};

   if (auto existing = _texture_load_tokens.find(name);
       existing != _texture_load_tokens.end()) {
      auto token = existing->second.lock();

      if (token) return token;
   }

   auto token = std::make_shared<const world_texture_load_token>();

   _texture_load_tokens.insert_or_assign(name, token);

   return token;
}

void texture_manager::update_textures() noexcept
{
   std::scoped_lock lock{_shared_mutex};

   for (auto it = _pending_textures.begin(); it != _pending_textures.end();) {
      auto elem_it = it++;
      auto& [name, pending_create] = *elem_it;

      if (pending_create.task.ready()) {
         auto texture = pending_create.task.get();

         _textures.insert_or_assign(name, texture_state{.texture = texture,
                                                        .asset = pending_create.asset});
         _copied_textures.insert_or_assign(name, texture);

         _pending_textures.erase(elem_it);
      }
   }
}

auto texture_manager::status(const lowercase_string& name,
                             const world_texture_dimension dimension) const noexcept
   -> texture_status
{
   std::shared_lock lock{_shared_mutex};

   if (auto it = _textures.find(name); it != _textures.end()) {
      const texture_state& state = it->second;

      if (auto texture = state.texture.lock(); texture) {
         if (texture->dimension != dimension) {
            return texture_status::dimension_mismatch;
         }

         return texture_status::ready;
      }
   }

   if (_pending_textures.contains(name)) return texture_status::loading;
   if (_copied_textures.contains(name)) return texture_status::loading;

   return texture_status::missing;
}

void texture_manager::init_texture(gpu::resource_handle texture,
                                   const assets::texture::texture& cpu_texture)
{
   pooled_copy_command_list command_list = _copy_command_list_pool.aquire_and_reset();

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

      command_list->copy_buffer_to_texture(texture, i, 0, 0, 0, upload_buffer.get(),
                                           {.offset = cpu_subresource.offset(),
                                            .format = cpu_subresource.dxgi_format(),
                                            .width = cpu_subresource.width(),
                                            .height = cpu_subresource.height(),
                                            .depth = 1,
                                            .row_pitch = cpu_subresource.row_pitch()});
   }

   command_list->close();

   _device.background_copy_queue.execute_command_lists(command_list.get());
}

void texture_manager::create_texture_async(const lowercase_string& name,
                                           asset_ref<assets::texture::texture> asset,
                                           asset_data<assets::texture::texture> data)
{
   pending_texture& pending = _pending_textures[name];

   if (pending.task.valid()) pending.task.cancel();

   pending = {.task = _thread_pool->exec(
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

                    return std::make_shared<world_texture>(
                       _device, std::move(texture), data->dxgi_format(),
                       data->flags().cube_map ? world_texture_dimension::cube
                                              : world_texture_dimension::_2d);
                 }),
              .asset = asset};
}

void texture_manager::texture_loaded(const lowercase_string& name,
                                     asset_ref<assets::texture::texture> asset,
                                     asset_data<assets::texture::texture> data) noexcept
{
   std::scoped_lock lock{_shared_mutex};

   create_texture_async(name, asset, data);
}

}