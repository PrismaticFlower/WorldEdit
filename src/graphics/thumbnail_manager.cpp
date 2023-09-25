#include "thumbnail_manager.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/odf/definition.hpp"
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
        _error_output{_error_output},
        _device{init.device}
   {
      const uint32 atlas_items_width = max_texture_length / temp_length;
      const uint32 atlas_items_height =
         (atlas_thumbnails + (atlas_items_width - 1)) / atlas_items_width;

      _atlas_texture = {_device.create_texture(
                           {.dimension = gpu::texture_dimension::t_2d,
                            .flags = {.allow_render_target = true},
                            .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                            .width = atlas_items_width * temp_length,
                            .height = atlas_items_height * temp_length,
                            .optimized_clear_value = {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                                      .color = {0.0f, 0.0f, 0.0f, 0.0f}},
                            .debug_name = "Thumbnails Render Target"},
                           gpu::barrier_layout::direct_queue_shader_resource,
                           gpu::legacy_resource_state::pixel_shader_resource),
                        _device.direct_queue};
      _atlas_srv = {_device.create_shader_resource_view(_atlas_texture.get(),
                                                        {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB}),
                    _device.direct_queue};
      _atlas_rtv = {_device.create_render_target_view(_atlas_texture.get(),
                                                      {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                                       .dimension =
                                                          gpu::rtv_dimension::texture2d}),
                    _device.direct_queue};

      _free_items.reserve(atlas_items_width * atlas_items_height);

      for (uint16 y = 0; y < atlas_items_height; ++y) {
         for (uint16 x = 0; x < atlas_items_width; ++x) {
            _free_items.push_back({x, y});
         }
      }

      _atlas_items_width = static_cast<float>(atlas_items_width);
      _atlas_items_height = static_cast<float>(atlas_items_height);
   }

   auto request_object_class_thumbnail(const std::string_view name) -> object_class_thumbnail
   {
      if (auto it = _items.find(name); it != _items.end()) {
         auto [x, y] = it->second;

         return {.imgui_texture_id = ImGui::GetFont()->ContainerAtlas->TexID,
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

   void draw_updated(model_manager& model_manager)
   {
      if (not _rendering) {
         std::scoped_lock lock{_pending_render_mutex};

         if (_pending_render.empty()) return;

         const auto& [name, pending] = *_pending_render.begin();

         _rendering = rendering{.name = name,
                                .model_name = pending.model_name,
                                .model = pending.model};
      }

      [[maybe_unused]] const model& model = model_manager[_rendering->model_name];

      // TODO: Render the thumbnail and stuff.
      _items.emplace(_rendering->name,
                     _free_items[_items.size() % _free_items.size()]);

      {
         std::scoped_lock lock{_pending_render_mutex};

         _pending_render.erase(_rendering->name);
         _rendering = std::nullopt;
      }
   }

private:
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
   gpu::unique_rtv_handle _atlas_rtv;

   struct rendering {
      std::string name;
      lowercase_string model_name;
      asset_ref<assets::msh::flat_model> model;
   };

   std::optional<rendering> _rendering;

   struct thumbnail_index {
      uint16 x = 0;
      uint16 y = 0;
   };

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
         [this](const auto&... args) { odf_loaded(args...); });
};

auto thumbnail_manager::request_object_class_thumbnail(const std::string_view name)
   -> object_class_thumbnail
{
   return _impl->request_object_class_thumbnail(name);
}

void thumbnail_manager::draw_updated(model_manager& model_manager)
{
   return _impl->draw_updated(model_manager);
}

thumbnail_manager::thumbnail_manager(const thumbnail_manager_init& init)
   : _impl{init}
{
}

thumbnail_manager::~thumbnail_manager() = default;

}