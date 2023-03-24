#include "interaction_context.hpp"

namespace we::world {

bool is_selected(const selected_entity entity, const interaction_targets& targets) noexcept
{
   for (const auto& selected : targets.selection) {
      if (selected == entity) return true;
   }

   return false;
}

bool is_selected(const path_id entity, const interaction_targets& targets) noexcept
{
   for (const auto& selected : targets.selection) {
      if (not std::holds_alternative<path_id_node_pair>(selected)) continue;

      if (std::get<path_id_node_pair>(selected).id == entity) return true;
   }

   return false;
}

}