#pragma once

#include "object_class_thumbnail.hpp"
#include "world/active_elements.hpp"
#include "world/interaction_context.hpp"
#include "world/object_class.hpp"
#include "world/tool_visualizers.hpp"

#include <memory>

#include <absl/container/flat_hash_map.h>

namespace we {

class output_stream;

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

struct renderer_init {
   const window_handle window;
   const std::shared_ptr<async::thread_pool>& thread_pool;
   assets::libraries_manager& asset_libraries;
   output_stream& error_output;

   float display_scale = 1.0f;

   bool use_debug_layer = false;
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

   virtual void wait_for_swap_chain_ready() = 0;

   virtual void draw_frame(const camera& camera, const world::world& world,
                           const world::interaction_targets& interaction_targets,
                           const world::active_entity_types active_entity_types,
                           const world::active_layers active_layers,
                           const world::tool_visualizers& tool_visualizers,
                           const world::object_class_library& world_classes,
                           const settings::graphics& settings) = 0;

   virtual auto draw_env_map(const env_map_params& params, const world::world& world,
                             const world::active_layers active_layers,
                             const world::object_class_library& world_classes)
      -> env_map_result = 0;

   virtual void window_resized(uint16 width, uint16 height) = 0;

   virtual void display_scale_changed(const float display_scale) = 0;

   virtual void recreate_imgui_font_atlas() = 0;

   virtual void reload_shaders() noexcept = 0;

   virtual auto request_object_class_thumbnail(const std::string_view name)
      -> object_class_thumbnail = 0;

   virtual void async_save_thumbnail_disk_cache(const wchar_t* path) noexcept = 0;

   virtual void async_load_thumbnail_disk_cache(const wchar_t* path) noexcept = 0;

   virtual void reset_thumbnails() noexcept = 0;
};

auto make_renderer(const renderer_init& init) -> std::unique_ptr<renderer>;

}
