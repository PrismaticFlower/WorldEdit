#pragma once

#include "world.hpp"

#include <span>
#include <variant>
#include <vector>

namespace we::world {

/// @brief A reference to a path node.
struct path_id_node_pair {
   path_id id;
   std::size_t node_index;

   bool operator==(const path_id_node_pair&) const noexcept = default;
};

/// @brief Represents an entity being interacted with (hovered or selected).
using interaction_target =
   std::variant<object_id, light_id, path_id_node_pair, region_id, sector_id, portal_id,
                hintnode_id, barrier_id, planning_hub_id, planning_connection_id, boundary_id>;

/// @brief Represents an entity being hovered over.
using hovered_entity = interaction_target;

/// @brief Represents an entity that is currently selected.
using selected_entity = interaction_target;

using creation_entity =
   std::variant<object, light, path, region, sector, portal, barrier, hintnode,
                planning_hub, planning_connection, boundary>;

/// @brief Stores references to the entities currently being interacted with.
struct interaction_targets {
   std::optional<hovered_entity> hovered_entity;
   std::vector<selected_entity> selection;

   std::optional<creation_entity> creation_entity;
};

/// @brief References to data that is managed by an edit stack.
struct edit_context {
   world& world;
   std::optional<creation_entity>& creation_entity;
   float3 euler_rotation;
   float3 light_region_euler_rotation;
};

bool is_selected(const selected_entity entity, const interaction_targets& targets) noexcept;

bool is_selected(const path_id entity, const interaction_targets& targets) noexcept;

}
