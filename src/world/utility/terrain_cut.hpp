#pragma once

#include "../active_elements.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include <span>

namespace we::world {

bool point_inside_terrain_cut(const float3 point, const float3 ray_direction,
                              const active_layers active_layers,
                              std::span<const object> objects,
                              const object_class_library& object_classes) noexcept;

auto gather_terrain_cuts(std::span<const object> objects,
                         const object_class_library& object_classes)
   -> std::vector<terrain_cut>;

}