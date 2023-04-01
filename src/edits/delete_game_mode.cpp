#include "delete_game_mode.hpp"

#include "utility/string_icompare.hpp"

#include <fmt/core.h>

namespace we::edits {

namespace {

struct delete_entry_req {
   int list_index = 0;
   int entry_index = 0;
   std::string entry;
};

auto make_delete_entries(const std::string_view game_mode_file_name,
                         const std::span<const world::requirement_list> requirements)
   -> std::vector<delete_entry_req>
{
   std::size_t count = 0;

   for (const auto& list : requirements) {
      if (not string::iequals(list.file_type, "lvl")) continue;

      for (const auto& entry : list.entries) {
         if (string::iequals(entry, game_mode_file_name)) count += 1;
      }
   }

   std::vector<delete_entry_req> entries;
   entries.reserve(count);

   for (int list_index = 0; list_index < requirements.size(); ++list_index) {
      const auto& list = requirements[list_index];

      if (not string::iequals(list.file_type, "lvl")) continue;

      int delete_offset = 0;

      for (int i = 0; i < list.entries.size(); ++i) {
         if (string::iequals(list.entries[i], game_mode_file_name)) {
            entries.emplace_back(list_index, i - delete_offset, list.entries[i]);
            delete_offset += 1;
         }
      }
   }

   return entries;
}

struct delete_game_mode final : edit<world::edit_context> {
   delete_game_mode(int index, world::game_mode_description game_mode,
                    std::vector<delete_entry_req> delete_requirements)
      : _index{index}, _game_mode{std::move(game_mode)}, _delete_requirements{std::move(delete_requirements)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      world::world& world = context.world;

      world.game_modes.erase(world.game_modes.begin() + _index);

      for (const auto& [list_index, entry_index, entry] : _delete_requirements) {
         auto& list = world.requirements[list_index];

         list.entries.erase(list.entries.begin() + entry_index);
      }
   }

   void revert(world::edit_context& context) const noexcept override
   {
      world::world& world = context.world;

      world.game_modes.insert(world.game_modes.begin() + _index, _game_mode);

      for (std::ptrdiff_t i = (std::ssize(_delete_requirements) - 1); i >= 0; --i) {
         const auto& [list_index, entry_index, entry] = _delete_requirements[i];
         auto& list = world.requirements[list_index];

         list.entries.emplace(list.entries.begin() + entry_index, entry);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const int _index;
   const world::game_mode_description _game_mode;

   const std::vector<delete_entry_req> _delete_requirements;
};

}

auto make_delete_game_mode(int game_mode_index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_game_mode>(
      game_mode_index, world.game_modes[game_mode_index],
      make_delete_entries(fmt::format("{}_{}", world.name,
                                      world.game_modes[game_mode_index].name),
                          world.requirements));
}

}