#pragma once

#include "world/active_elements.hpp"
#include "world/interaction_context.hpp"
#include "world/object_class.hpp"

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

}

namespace we::graphics {

class camera;

struct renderer {
   using window_handle = void*;

   virtual ~renderer() = default;

   virtual void wait_for_swap_chain_ready() = 0;

   virtual void draw_frame(const camera& camera, const world::world& world,
                           const world::interaction_targets& interaction_targets,
                           const world::active_entity_types active_entity_types,
                           const world::active_layers active_layers,
                           const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
                           const settings::graphics& settings) = 0;

   virtual void window_resized(uint16 width, uint16 height) = 0;

   virtual void mark_dirty_terrain() noexcept = 0;

   virtual void recreate_imgui_font_atlas() = 0;

   virtual void reload_shaders() noexcept = 0;
};

auto make_renderer(const renderer::window_handle window,
                   std::shared_ptr<async::thread_pool> thread_pool,
                   assets::libraries_manager& asset_libraries,
                   output_stream& error_output) -> std::unique_ptr<renderer>;

}
