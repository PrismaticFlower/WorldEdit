#pragma once

#include "../interaction_context.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include <span>

namespace we::world {

auto selection_centre_for_rotate_around(const world& world,
                                        const std::span<const selected_entity> selection)
   -> float3;

auto selection_centre_for_env_map(const world& world,
                                  const std::span<const selected_entity> selection,
                                  const object_class_library& object_classes) -> float3;

}
