#pragma once

#include "fallback_imgui_texture.hpp"
#include "object_class_thumbnail.hpp"
#include "world/active_elements.hpp"
#include "world/interaction_context.hpp"
#include "world/object_class.hpp"
#include "world/tool_visualizers.hpp"

#include <memory>

namespace we {

class output_stream;
struct gizmo_draw_lists;

}

namespace we::async {

class thread_pool;

}

namespace we::assets {

struct libraries_manager;

}

namespace we::settings {

struct graphics;

}

namespace we::world {

struct world;
struct object_class_library;

}

namespace we::graphics {

class camera;
using window_handle = void*;
using waitable_handle = void*;

struct renderer_init {
   const window_handle window;
   const std::shared_ptr<async::thread_pool>& thread_pool;
   assets::libraries_manager& asset_libraries;
   output_stream& error_output;

   float display_scale = 1.0f;

   bool prefer_high_performance_gpu = false;
   bool use_debug_layer = false;
   bool use_legacy_barriers = false;
   bool never_use_shader_model_6_6 = false;
   bool never_use_open_existing_heap = false;
   bool never_use_write_buffer_immediate = false;
   bool never_use_relaxed_format_casting = false;
   bool never_use_target_independent_rasterization = false;
};

struct draw_frame_options {
   bool draw_terrain_grid = false;
   bool draw_overlay_grid = false;
   bool draw_foliage_map_overlay = false;
   float delta_time = 0.0f;
   float overlay_grid_height = 0.0f;
   float overlay_grid_size = 4.0f;
};

struct env_map_params {
   float3 positionWS;
   uint32 length = 512;
};

struct env_map_result {
   uint32 length = 512;
   uint32 row_pitch = 512;
   uint32 item_pitch = row_pitch * length;

   std::unique_ptr<std::byte[]> data;
};

struct renderer {
   virtual ~renderer() = default;

   virtual auto get_swap_chain_waitable_object() noexcept -> waitable_handle = 0;

   virtual void draw_frame(const camera& camera, const world::world& world,
                           const world::interaction_targets& interaction_targets,
                           const world::active_entity_types active_entity_types,
                           const world::active_layers active_layers,
                           const world::tool_visualizers& tool_visualizers,
                           const world::object_class_library& world_classes,
                           const gizmo_draw_lists& gizmo_draw_lists,
                           const draw_frame_options frame_options,
                           const settings::graphics& settings) = 0;

   virtual auto draw_env_map(const env_map_params& params, const world::world& world,
                             const world::active_entity_types active_entity_types,
                             const world::active_layers active_layers,
                             const world::object_class_library& world_classes)
      -> env_map_result = 0;

   virtual void window_resized(uint16 width, uint16 height) = 0;

   virtual void display_scale_changed(const float display_scale) = 0;

   virtual void reload_shaders() noexcept = 0;

   virtual auto request_object_class_thumbnail(const std::string_view name)
      -> object_class_thumbnail = 0;

   virtual auto request_texture_thumbnail(const std::string_view name)
      -> object_class_thumbnail = 0;

   virtual void async_save_thumbnail_disk_cache(const char* path) noexcept = 0;

   virtual void async_load_thumbnail_disk_cache(const char* path) noexcept = 0;

   virtual void reset_thumbnails() noexcept = 0;

   virtual auto request_imgui_texture_id(const std::string_view name,
                                         const fallback_imgui_texture fallback) noexcept
      -> uint32 = 0;

   virtual bool get_profiler_enabled() noexcept = 0;

   virtual void set_profiler_enabled(const bool enabled) noexcept = 0;

   const static float thumbnail_base_length;
};

auto make_renderer(const renderer_init& init) -> std::unique_ptr<renderer>;

}
