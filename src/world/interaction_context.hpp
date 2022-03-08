#pragma once

#include "world.hpp"

#include <span>
#include <vector>

#include <boost/variant2/variant.hpp>

namespace we::world {

/// @brief A reference to a path node.
struct path_id_node_pair {
   path_id id;
   std::size_t node_index;

   bool operator==(const path_id_node_pair&) const noexcept = default;
};

/// @brief Represents an entity being interacted with (hovered or selected).
using interaction_target =
   boost::variant2::variant<object_id, light_id, path_id, path_id_node_pair,
                            region_id, sector_id, portal_id, hintnode_id, barrier_id,
                            planning_hub_id, planning_connection_id, boundary_id>;

/// @brief Represents an entity being hovered over.
using hovered_entity = interaction_target;

/// @brief Represents an entity that is currently selected.
using selected_entity = interaction_target;

/// @brief Stores references to the entities currently being interacted with.
struct interaction_targets {
   std::optional<hovered_entity> hovered_entity;
   std::vector<selected_entity> selection;
};

}
