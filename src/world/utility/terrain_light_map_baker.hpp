#pragma once

#include "../active_elements.hpp"

#include "container/dynamic_array_2d.hpp"
#include "types.hpp"

#include <memory>

namespace we::async {

class thread_pool;

}

namespace we::world {

namespace detail {
struct terrain_light_map_baker_impl;
}

struct world;
struct object_class_library;

struct terrain_light_map_baker_config {
   bool include_object_shadows = true;
   bool ambient_occlusion = true;
   bool supersample = true;

   int32 ambient_occlusion_samples = 128;
};

struct terrain_light_map_baker {
   terrain_light_map_baker(const world& world, const object_class_library& library,
                           const active_layers active_layers,
                           async::thread_pool& thread_pool,
                           const terrain_light_map_baker_config& config) noexcept;

   ~terrain_light_map_baker();

   terrain_light_map_baker(const terrain_light_map_baker&) = delete;
   auto operator=(const terrain_light_map_baker&) -> terrain_light_map_baker& = delete;

   bool ready() const noexcept;

   auto progress() const noexcept -> float;

   auto light_map() noexcept -> container::dynamic_array_2d<uint32>;

private:
   std::unique_ptr<detail::terrain_light_map_baker_impl> _impl;
};

}