#pragma once

#include "game_mode_unlink_layer.hpp"
#include "utility/string_icompare.hpp"

#include <cassert>

#include <fmt/core.h>

namespace we::edits {

namespace {

enum class req_edit_type { none, append, create };

struct delete_entry_req {
   int list_index = 0;
   int entry_index = 0;
   std::string entry;
};

auto make_delete_req_entries(const std::string_view layer_file_name,
                             const std::span<const world::requirement_list> requirements)
   -> std::vector<delete_entry_req>
{
   std::size_t count = 0;

   for (const auto& list : requirements) {
      if (not string::iequals(list.file_type, "world")) continue;

      for (const auto& entry : list.entries) {
         if (string::iequals(entry, layer_file_name)) count += 1;
      }
   }

   std::vector<delete_entry_req> entries;
   entries.reserve(count);

   for (int list_index = 0; list_index < requirements.size(); ++list_index) {
      const auto& list = requirements[list_index];

      if (not string::iequals(list.file_type, "world")) continue;

      int delete_offset = 0;

      for (int i = 0; i < list.entries.size(); ++i) {
         if (string::iequals(list.entries[i], layer_file_name)) {
            entries.emplace_back(list_index, i - delete_offset, list.entries[i]);
            delete_offset += 1;
         }
      }
   }

   return entries;
}

struct unlink_layer final : edit<world::edit_context> {
   unlink_layer(int game_mode_index, int game_mode_layers_index, int layer_index,
                std::vector<delete_entry_req> delete_requirements)
      : _game_mode_index{game_mode_index},
        _game_mode_layers_index{game_mode_layers_index},
        _layer_index{layer_index},
        _delete_requirements{delete_requirements}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::world& world = context.world;
      world::game_mode_description& game_mode = world.game_modes[_game_mode_index];

      game_mode.layers.erase(game_mode.layers.begin() + _game_mode_layers_index);

      for (const auto& [list_index, entry_index, entry] : _delete_requirements) {
         auto& list = game_mode.requirements[list_index];

         list.entries.erase(list.entries.begin() + entry_index);
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::game_mode_description& game_mode =
         context.world.game_modes[_game_mode_index];

      game_mode.layers.insert(game_mode.layers.begin() + _game_mode_layers_index,
                              _layer_index);

      for (std::ptrdiff_t i = (std::ssize(_delete_requirements) - 1); i >= 0; --i) {
         const auto& [list_index, entry_index, entry] = _delete_requirements[i];
         auto& list = game_mode.requirements[list_index];

         list.entries.emplace(list.entries.begin() + entry_index, entry);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const int _game_mode_index;
   const int _game_mode_layers_index;
   const int _layer_index;
   const std::vector<delete_entry_req> _delete_requirements;
};

}

auto make_game_mode_unlink_layer(int game_mode_index, int game_mode_layer_index,
                                 const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const int layer_index =
      world.game_modes[game_mode_index].layers[game_mode_layer_index];
   const std::string layer_file_name =
      fmt::format("{}_{}", world.name, world.layer_descriptions[layer_index].name);

   return std::make_unique<unlink_layer>(
      game_mode_index, game_mode_layer_index, layer_index,
      game_mode_index != 0
         ? make_delete_req_entries(layer_file_name,
                                   world.game_modes[game_mode_index].requirements)
         : std::vector<delete_entry_req>{});
}

}
