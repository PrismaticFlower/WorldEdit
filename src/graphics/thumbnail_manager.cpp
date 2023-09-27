#include "thumbnail_manager.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/odf/definition.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "gpu/resource.hpp"
#include "model_manager.hpp"
#include "utility/string_icompare.hpp"

#include <optional>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <imgui.h>

using namespace std::literals;

namespace we::graphics {

namespace {

constexpr uint32 max_texture_length = 16384;
constexpr uint32 temp_length = 128;
constexpr uint32 atlas_thumbnails = 256;

}

struct thumbnail_manager::impl {
   explicit impl(const thumbnail_manager_init& init)
      : _asset_libraries{init.asset_libraries},
        _error_output{init.error_output},
        _device{init.device}
   {
      const uint32 atlas_items_width = max_texture_length / temp_length;
      const uint32 atlas_items_height =
         (atlas_thumbnails + (atlas_items_width - 1)) / atlas_items_width;

      _atlas_texture =
         {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                  .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                  .width = atlas_items_width * temp_length,
                                  .height = atlas_items_height * temp_length,
                                  .debug_name = "Thumbnails Atlas"},
                                 gpu::barrier_layout::direct_queue_common,
                                 gpu::legacy_resource_state::pixel_shader_resource),
          _device.direct_queue};
      _atlas_srv = {_device.create_shader_resource_view(_atlas_texture.get(),
                                                        {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB}),
                    _device.direct_queue};

      _free_items.reserve(atlas_items_width * atlas_items_height);

      for (uint16 y = 0; y < atlas_items_height; ++y) {
         for (uint16 x = 0; x < atlas_items_width; ++x) {
            _free_items.push_back({x, y});
         }
      }

      _atlas_items_width = static_cast<float>(atlas_items_width);
      _atlas_items_height = static_cast<float>(atlas_items_height);

      _render_texture = {_device.create_texture(
                            {.dimension = gpu::texture_dimension::t_2d,
                             .flags = {.allow_render_target = true},
                             .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                             .width = temp_length,
                             .height = temp_length,
                             .optimized_clear_value = {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                                       .color = {0.0f, 0.0f, 0.0f, 0.0f}},
                             .debug_name = "Thumbnails Render Target"},
                            gpu::barrier_layout::direct_queue_copy_source,
                            gpu::legacy_resource_state::copy_source),
                         _device.direct_queue};
      _render_rtv = {_device.create_render_target_view(_render_texture.get(),
                                                       {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                                        .dimension =
                                                           gpu::rtv_dimension::texture2d}),
                     _device.direct_queue};

      _depth_texture = {_device.create_texture(
                           {.dimension = gpu::texture_dimension::t_2d,
                            .flags = {.allow_depth_stencil = true,
                                      .deny_shader_resource = true},
                            .format = DXGI_FORMAT_D16_UNORM,
                            .width = temp_length,
                            .height = temp_length,
                            .optimized_clear_value = {.format = DXGI_FORMAT_D16_UNORM,
                                                      .depth_stencil = {.depth = 1.0f}},
                            .debug_name = "Thumbnails Depth Buffer"},
                           gpu::barrier_layout::depth_stencil_write,
                           gpu::legacy_resource_state::depth_write),
                        _device.direct_queue};
      _depth_dsv = {_device.create_depth_stencil_view(_depth_texture.get(),
                                                      {.format = DXGI_FORMAT_D16_UNORM,
                                                       .dimension =
                                                          gpu::dsv_dimension::texture2d}),
                    _device.direct_queue};
   }

   auto request_object_class_thumbnail(const std::string_view name) -> object_class_thumbnail
   {
      if (auto it = _items.find(name); it != _items.end()) {
         auto [x, y] = it->second;

         return {.imgui_texture_id =
                    reinterpret_cast<void*>(uint64{_atlas_srv.get().index}),
                 .uv_left = x / _atlas_items_width,
                 .uv_top = y / _atlas_items_height,
                 .uv_right = (x + 1) / _atlas_items_width,
                 .uv_bottom = (y + 1) / _atlas_items_height};
      }

      enqueue_create_thumbnail(name);

      return {.imgui_texture_id = ImGui::GetFont()->ContainerAtlas->TexID,
              .uv_left = 0.0f,
              .uv_top = 0.0f,
              .uv_right = 1.0f,
              .uv_bottom = 1.0f};
   }

   void draw_updated(model_manager& model_manager,
                     root_signature_library& root_signature_library,
                     pipeline_library& pipeline_library,
                     dynamic_buffer_allocator& dynamic_buffer_allocator,
                     gpu::graphics_command_list& command_list)
   {
      if (not _rendering) {
         std::scoped_lock lock{_pending_render_mutex};

         if (_pending_render.empty()) return;

         const auto& [name, pending] = *_pending_render.begin();

         const thumbnail_index thumbnail_index =
            _items
               .emplace(name, _free_items[_items.size() % _free_items.size()])
               .first->second;

         _rendering = rendering{.name = name,
                                .model_name = pending.model_name,
                                .model = pending.model,
                                .thumbnail_index = thumbnail_index};
      }

      const model& model = model_manager[_rendering->model_name];
      const model_status status = model_manager.status(_rendering->model_name);

      if (status == model_status::ready or status == model_status::ready_textures_missing or
          status == model_status::ready_textures_loading) {
         draw(model, _rendering->thumbnail_index, root_signature_library,
              pipeline_library, dynamic_buffer_allocator, command_list);

         if (status == model_status::ready or
             status == model_status::ready_textures_missing) {
            std::scoped_lock lock{_pending_render_mutex};

            _pending_render.erase(_rendering->name);
            _rendering = std::nullopt;
         }
      }
      else if (status == model_status::errored or status == model_status::missing) {
         _error_output.write("Unable to render thumbnail for '{}'. Model "
                             "errored or missing.\n",
                             _rendering->name);

         std::scoped_lock lock{_pending_render_mutex, _missing_model_odfs_mutex};

         _missing_model_odfs.emplace(_rendering->name);
         _pending_render.erase(_rendering->name);
         _rendering = std::nullopt;
      }
   }

private:
   struct thumbnail_index {
      uint16 x = 0;
      uint16 y = 0;
   };

   void draw(const model& model, const thumbnail_index index,
             root_signature_library& root_signature_library,
             pipeline_library& pipeline_library,
             dynamic_buffer_allocator& dynamic_buffer_allocator,
             gpu::graphics_command_list& command_list)
   {
      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::none,
                                 .sync_after = gpu::barrier_sync::render_target,
                                 .access_before = gpu::barrier_access::no_access,
                                 .access_after = gpu::barrier_access::render_target,
                                 .layout_before = gpu::barrier_layout::direct_queue_copy_source,
                                 .layout_after = gpu::barrier_layout::render_target,
                                 .resource = _render_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _render_texture.get(),
            .state_before = gpu::legacy_resource_state::copy_source,
            .state_after = gpu::legacy_resource_state::render_target});
      }

      command_list.flush_barriers();

      command_list.clear_render_target_view(_render_rtv.get(),
                                            float4{0.0f, 0.0f, 0.0f, 0.0f});
      command_list.clear_depth_stencil_view(_depth_dsv.get(),
                                            {.clear_depth = true}, 1.0f, 0x0);

      command_list.rs_set_viewports(
         gpu::viewport{.width = static_cast<float>(temp_length),
                       .height = static_cast<float>(temp_length)});
      command_list.rs_set_scissor_rects({.right = temp_length, .bottom = temp_length});
      command_list.om_set_render_targets(_render_rtv.get(), _depth_dsv.get());

      (void)model, root_signature_library, pipeline_library, dynamic_buffer_allocator;

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::render_target,
                                 .sync_after = gpu::barrier_sync::copy,
                                 .access_before = gpu::barrier_access::render_target,
                                 .access_after = gpu::barrier_access::copy_source,
                                 .layout_before = gpu::barrier_layout::render_target,
                                 .layout_after = gpu::barrier_layout::direct_queue_copy_source,
                                 .resource = _render_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _render_texture.get(),
            .state_before = gpu::legacy_resource_state::render_target,
            .state_after = gpu::legacy_resource_state::copy_source});
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _atlas_texture.get(),
            .state_before = gpu::legacy_resource_state::pixel_shader_resource,
            .state_after = gpu::legacy_resource_state::copy_dest});
      }

      command_list.flush_barriers();

      const gpu::rect atlas_rect{.left = index.x * temp_length,
                                 .top = index.y * temp_length,
                                 .right = (index.x + 1) * temp_length,
                                 .bottom = (index.y + 1) * temp_length};

      command_list.copy_texture_region(_atlas_texture.get(), 0, atlas_rect.left,
                                       atlas_rect.top, 0, _render_texture.get(), 0);

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::copy,
                                 .sync_after = gpu::barrier_sync::pixel_shading,
                                 .access_before = gpu::barrier_access::copy_dest,
                                 .access_after = gpu::barrier_access::shader_resource,
                                 .layout_before = gpu::barrier_layout::direct_queue_common,
                                 .layout_after = gpu::barrier_layout::direct_queue_common,
                                 .resource = _atlas_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _atlas_texture.get(),
            .state_before = gpu::legacy_resource_state::copy_dest,
            .state_after = gpu::legacy_resource_state::pixel_shader_resource});
      }
   }

   void enqueue_create_thumbnail(const std::string_view name)
   {
      {
         std::scoped_lock lock{_missing_model_odfs_mutex, _pending_render_mutex};

         if (_missing_model_odfs.contains(name)) return;
         if (_pending_render.contains(name)) return;
      }

      asset_ref odf_asset_ref = _asset_libraries.odfs[lowercase_string{name}];

      if (not odf_asset_ref.exists()) return;

      asset_data odf_data = odf_asset_ref.get_if();

      if (odf_data) {
         odf_loaded(name, odf_asset_ref, odf_data);
      }
      else {
         std::scoped_lock lock{_pending_odfs_mutex};

         _pending_odfs.emplace(std::string{name}, std::move(odf_asset_ref));
      }
   }

   void odf_loaded(const std::string_view name, asset_ref<assets::odf::definition> ref,
                   asset_data<assets::odf::definition> definition) noexcept
   {
      const lowercase_string model_name = [&] {
         if (string::iends_with(definition->header.geometry_name, ".msh"sv)) {
            return lowercase_string{definition->header.geometry_name.substr(
               0, definition->header.geometry_name.size() - ".msh"sv.size())};
         }
         else {
            return lowercase_string{definition->header.geometry_name};
         }
      }();

      if (model_name.empty()) {
         std::scoped_lock lock{_missing_model_odfs_mutex};

         _missing_model_odfs.emplace(name);

         return;
      }

      std::scoped_lock lock{_pending_render_mutex, _pending_odfs_mutex,
                            _missing_model_odfs_mutex};

      _pending_odfs.erase(name);
      _pending_render.emplace(name,
                              pending_render{model_name,
                                             _asset_libraries.models[model_name]});
      _missing_model_odfs.erase(name);
   }

   assets::libraries_manager& _asset_libraries;
   output_stream& _error_output;
   gpu::device& _device;

   float _atlas_items_width = 1;
   float _atlas_items_height = 1;

   gpu::unique_resource_handle _atlas_texture;
   gpu::unique_resource_view _atlas_srv;

   gpu::unique_resource_handle _render_texture;
   gpu::unique_rtv_handle _render_rtv;

   gpu::unique_resource_handle _depth_texture;
   gpu::unique_dsv_handle _depth_dsv;

   struct rendering {
      std::string name;
      lowercase_string model_name;
      asset_ref<assets::msh::flat_model> model;
      thumbnail_index thumbnail_index;
   };

   std::optional<rendering> _rendering;

   std::vector<thumbnail_index> _free_items;
   absl::flat_hash_map<std::string, thumbnail_index> _items;

   std::shared_mutex _pending_odfs_mutex;
   absl::flat_hash_map<std::string, asset_ref<assets::odf::definition>> _pending_odfs;

   struct pending_render {
      lowercase_string model_name;
      asset_ref<assets::msh::flat_model> model;
   };

   std::shared_mutex _pending_render_mutex;
   absl::flat_hash_map<std::string, pending_render> _pending_render;

   std::shared_mutex _missing_model_odfs_mutex;
   absl::flat_hash_set<std::string> _missing_model_odfs;

   event_listener<void(const lowercase_string&, asset_ref<assets::odf::definition>,
                       asset_data<assets::odf::definition>)>
      _asset_load_listener = _asset_libraries.odfs.listen_for_loads(
         [this](const lowercase_string& name,
                const asset_ref<assets::odf::definition>& ref,
                const asset_data<assets::odf::definition>& data) {
            {
               std::shared_lock lock{_pending_odfs_mutex};

               if (not _pending_odfs.contains(name)) return;
            }

            odf_loaded(name, ref, data);
         });
};

auto thumbnail_manager::request_object_class_thumbnail(const std::string_view name)
   -> object_class_thumbnail
{
   return _impl->request_object_class_thumbnail(name);
}

void thumbnail_manager::draw_updated(model_manager& model_manager,
                                     root_signature_library& root_signature_library,
                                     pipeline_library& pipeline_library,
                                     dynamic_buffer_allocator& dynamic_buffer_allocator,
                                     gpu::graphics_command_list& command_list)
{
   return _impl->draw_updated(model_manager, root_signature_library, pipeline_library,
                              dynamic_buffer_allocator, command_list);
}

thumbnail_manager::thumbnail_manager(const thumbnail_manager_init& init)
   : _impl{init}
{
}

thumbnail_manager::~thumbnail_manager() = default;
}