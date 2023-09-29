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

   bool use_debug_layer = false;
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

   virtual void window_resized(uint16 width, uint16 height) = 0;

   virtual void display_scale_changed(const float display_scale) noexcept = 0;

   virtual void mark_dirty_terrain() noexcept = 0;

   virtual void recreate_imgui_font_atlas() = 0;

   virtual void reload_shaders() noexcept = 0;

   virtual auto request_object_class_thumbnail(const std::string_view name)
      -> object_class_thumbnail = 0;
};

auto make_renderer(const renderer_init& init) -> std::unique_ptr<renderer>;

}
