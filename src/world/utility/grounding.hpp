#pragma once

#include "../active_elements.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include <optional>

namespace we::world {

struct blocks_custom_mesh_bvh_library;

/// @brief Get the grounded position of an object. Or nullopt if the object is already grounded or can't be grounded.
/// @param object The object to ground.
/// @param world The world.
/// @param object_classes The object classes to use.
/// @param blocks_bvh_library The BVH library to use custom mesh blocks.
/// @param active_layers The active layers to raycast against.
/// @param terrain_collision The terrain collision.
/// @return The grounded position of the object or nullopt.
auto ground_object(const object& object, const world& world,
                   const object_class_library& object_classes,
                   const blocks_custom_mesh_bvh_library& blocks_bvh_library,
                   const active_layers active_layers) noexcept
   -> std::optional<float3>;

/// @brief Get the grounded position of a light. Or nullopt if the light is already grounded or can't be grounded.
/// @param light The light to ground.
/// @param world The world.
/// @param object_classes The object classes to use.
/// @param blocks_bvh_library The BVH library to use custom mesh blocks.
/// @param active_layers The active layers to raycast against.
/// @param terrain_collision The terrain collision.
/// @return The grounded position of the light or nullopt.
auto ground_light(const light& light, const world& world,
                  const object_class_library& object_classes,
                  const blocks_custom_mesh_bvh_library& blocks_bvh_library,
                  const active_layers active_layers) noexcept -> std::optional<float3>;

/// @brief Get the grounded position of a region. Or nullopt if the region is already grounded or can't be grounded.
/// @param region The region to ground.
/// @param world The world.
/// @param object_classes The object classes to use.
/// @param blocks_bvh_library The BVH library to use custom mesh blocks.
/// @param active_layers The active layers to raycast against.
/// @param terrain_collision The terrain collision.
/// @return The grounded position of the region or nullopt.
auto ground_region(const region& region, const world& world,
                   const object_class_library& object_classes,
                   const blocks_custom_mesh_bvh_library& blocks_bvh_library,
                   const active_layers active_layers) noexcept
   -> std::optional<float3>;

/// @brief Get the grounded base of a sector. Or nullopt if the sector is already grounded or can't be grounded.
/// @param sector The sector to ground.
/// @param world The world.
/// @param object_classes The object classes to use.
/// @param blocks_bvh_library The BVH library to use custom mesh blocks.
/// @param active_layers The active layers to raycast against.
/// @param terrain_collision The terrain collision.
/// @return The grounded base of the sector or nullopt.
auto ground_sector(const sector& sector, const world& world,
                   const object_class_library& object_classes,
                   const blocks_custom_mesh_bvh_library& blocks_bvh_library,
                   const active_layers active_layers) noexcept -> std::optional<float>;

/// @brief Get the grounded base of a portal. Or nullopt if the portal is already grounded or can't be grounded.
/// @param portal The portal to ground.
/// @param world The world.
/// @param object_classes The object classes to use.
/// @param blocks_bvh_library The BVH library to use custom mesh blocks.
/// @param active_layers The active layers to raycast against.
/// @param terrain_collision The terrain collision.
/// @return The grounded base of the portal or nullopt.
auto ground_portal(const portal& portal, const world& world,
                   const object_class_library& object_classes,
                   const blocks_custom_mesh_bvh_library& blocks_bvh_library,
                   const active_layers active_layers) noexcept
   -> std::optional<float3>;

/// @brief Get the grounded position of a point. Or nullopt if the point is already grounded or can't be grounded.
/// @param point The point to ground.
/// @param world The world.
/// @param object_classes The object classes to use.
/// @param blocks_bvh_library The BVH library to use custom mesh blocks.
/// @param active_layers The active layers to raycast against.
/// @param terrain_collision The terrain collision.
/// @return The grounded position of the point or nullopt.
auto ground_point(const float3 point, const world& world,
                  const object_class_library& object_classes,
                  const blocks_custom_mesh_bvh_library& blocks_bvh_library,
                  const active_layers active_layers) noexcept -> std::optional<float3>;

}