#pragma once

#include "../active_elements.hpp"
#include "container/dynamic_array_2d.hpp"
#include "types.hpp"

namespace we::world {

struct world;
struct object_class_library;

auto bake_terrain_light_map(const world& world, const object_class_library& library,
                            const active_layers active_layers) noexcept
   -> container::dynamic_array_2d<uint32>;

}