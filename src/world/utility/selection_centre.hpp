#pragma once

#include "../interaction_context.hpp"
#include "../world.hpp"

#include <span>

namespace we::world {

auto selection_centre_for_rotate_around(const world& world,
                                        const std::span<const selected_entity> selection)
   -> float3;

}
