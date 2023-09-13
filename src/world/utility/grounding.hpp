#pragma once

#include "../active_elements.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include <optional>

namespace we::world {

/// @brief Get the grounded position of an object. Or nullopt if the object is already grounded or can't be grounded.
/// @param obj The object to ground.
/// @param world The world.
/// @param object_classes The object classes to use.
/// @param active_layers The active layers to raycast against.
/// @param terrain_collision The terrain collision.
/// @return The grounded position of an object or nullopt.
auto ground_object(const object& obj, const world& world,
                   const object_class_library& object_classes,
                   const active_layers active_layers,
                   const terrain_collision& terrain_collision)
   -> std::optional<float3>;

}