#pragma once

#include "../entity_group.hpp"
#include "../interaction_context.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"
#include "math/bounding_box.hpp"

#include <span>
#include <string_view>

namespace we::world {

struct entity_group_placement_metrics {
   float ground_distance;
   math::bounding_box visual_bbox;
};

/// @brief Return the placement metrics for the entity group.
/// @param group The entity group.
/// @param object_classes The object class library.
/// @return The metrics.
auto entity_group_metrics(const entity_group& group,
                          const object_class_library& object_classes) noexcept
   -> entity_group_placement_metrics;

/// @brief Centre the entity group on 0,0,0.
/// @param group The entity group.
void centre_entity_group(entity_group& group) noexcept;

/// @brief Returns the new name of an object after it is was placed into the world.
/// @param name The original name of the object in the entity group.
/// @param world_objects The span of world objects.
/// @param group The entity group the object is from.
/// @param group_base_index The index of the first object inside world_objects.
/// @return The name of the entity after it was placed or name on failure.
auto get_placed_entity_name(std::string_view name, std::span<const object> world_objects,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view;

/// @brief Returns the new name of an path after it is was placed into the world.
/// @param name The original name of the path in the entity group.
/// @param world_paths The span of world paths.
/// @param group The entity group the path is from.
/// @param group_base_index The index of the first path inside world_paths.
/// @return The name of the entity after it was placed or name on failure.
auto get_placed_entity_name(std::string_view name, std::span<const path> world_paths,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view;

/// @brief Returns the new name of an sector after it is was placed into the world.
/// @param name The original name of the sector in the entity group.
/// @param world_sectors The span of world sectors.
/// @param group The entity group the sector is from.
/// @param group_base_index The index of the first sector inside world_sectors.
/// @return The name of the entity after it was placed or name on failure.
auto get_placed_entity_name(std::string_view name, std::span<const sector> world_sectors,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view;

/// @brief Make an entity_group from a selection.
/// @param world The world.
/// @param selection The selection.
/// @return The entity group.
auto make_entity_group_from_selection(const world& world, const selection& selection) noexcept
   -> entity_group;

/// @brief Make an entity_group from a block_id.
/// @param blocks The world blocks.
/// @param selection The selection.
/// @return The entity group.
auto make_entity_group_from_block_id(const blocks& blocks, const block_id id) noexcept
   -> entity_group;

/// @brief Make an entity_group that includes all entities from a layer.
/// @param world The world.
/// @param layer The index of the layer.
/// @return The entity group.
auto make_entity_group_from_layer(const world& world, const int32 layer) noexcept
   -> entity_group;

/// @brief Make an entity_group from a world.
/// @param world The world.
/// @return The entity group.
auto make_entity_group_from_world(const world& world) noexcept -> entity_group;

/// @brief Check if an entity_group is empty.
/// @param group The group.
/// @return True if empty, false otherwise.
bool is_entity_group_empty(const entity_group& group) noexcept;

/// @brief Check if an entity_group's blocks are empty.
/// @param group The group.
/// @return True if empty, false otherwise.
bool is_entity_group_blocks_empty(const entity_group& group) noexcept;

}