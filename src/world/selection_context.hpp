#pragma once

#include <span>
#include <vector>

namespace we::world {

/// @brief Classifies the type of the selected item.
enum class selection_type {
   object,
   light,
   path,
   region,
   sector,
   portal,
   hintnode,
   barrier,
   planning_hub,
   planning_connection,
   boundary
};

/// @brief Represents a selected item.
struct selection_item {
   /// @brief Type of the selected item.
   selection_type type;

   /// @brief The index of the selected item inside the world struct.
   std::size_t index;
};

/// @brief Represents a selection.
using selection_context = std::vector<selection_item>;

/// @brief Represents a constant view over a selection.
using selection_context_view = std::span<const selection_item>;

}
