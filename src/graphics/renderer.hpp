#pragma once

#include "world/active_elements.hpp"
#include "world/interaction_context.hpp"
#include "world/object_class.hpp"

#include <memory>

#include <absl/container/flat_hash_map.h>

namespace we::settings {

class graphics;

}

namespace we::async {

class thread_pool;

}

namespace we::assets {

class libraries_manager;

}

namespace we::settings {

class graphics;

}

namespace we::world {

struct world;

}

namespace we::graphics {

class renderer_impl;
class camera;

class renderer {
public:
   renderer(const HWND window, std::shared_ptr<settings::graphics> settings,
            std::shared_ptr<async::thread_pool> thread_pool,
            assets::libraries_manager& asset_libraries, output_stream& error_output);

   ~renderer();

   void draw_frame(const camera& camera, const world::world& world,
                   const world::interaction_targets& interaction_targets,
                   const world::active_entity_types active_entity_types,
                   const world::active_layers active_layers,
                   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes);

   void window_resized(uint16 width, uint16 height);

   void mark_dirty_terrain() noexcept;

private:
   std::unique_ptr<renderer_impl> _impl;
};

}
