#include "rename_layer.hpp"
#include "utility/string_icompare.hpp"

#include <optional>

#include <fmt/core.h>

namespace we::edits {

namespace {

struct req_entry_edit {
   uint32 list_index = 0;
   uint32 entry_index = 0;

   std::string value;
};

struct game_mode_req_entry_edit {
   uint32 game_mode_index = 0;
   uint32 list_index = 0;
   uint32 entry_index = 0;

   std::string value;
};

auto make_req_entry_edit(std::string_view new_value, std::string_view old_value,
                         std::span<const world::requirement_list> req_list) noexcept
   -> std::optional<req_entry_edit>
{
   for (uint32 list_index = 0; list_index < req_list.size(); ++list_index) {
      for (uint32 entry_index = 0;
           entry_index < req_list[list_index].entries.size(); ++entry_index) {
         if (not string::iequals("world", req_list[list_index].file_type)) {
            continue;
         }

         if (string::iequals(req_list[list_index].entries[entry_index], old_value)) {
            return req_entry_edit{.list_index = list_index,
                                  .entry_index = entry_index,
                                  .value = std::string{new_value}};
         }
      }
   }

   return std::nullopt;
}

auto make_game_mode_req_entry_edits(std::string_view new_value,
                                    std::string_view old_value,
                                    std::span<const world::game_mode_description> game_modes) noexcept
   -> std::vector<game_mode_req_entry_edit>
{
   std::size_t count = 0;

   for (uint32 game_mode_index = 0; game_mode_index < game_modes.size();
        ++game_mode_index) {
      std::span<const world::requirement_list> requirements =
         game_modes[game_mode_index].requirements;

      for (uint32 list_index = 0; list_index < requirements.size(); ++list_index) {
         const world::requirement_list& req_list = requirements[list_index];

         if (not string::iequals("world", req_list.file_type)) {
            continue;
         }

         for (uint32 entry_index = 0; entry_index < req_list.entries.size();
              ++entry_index) {
            if (string::iequals(req_list.entries[entry_index], old_value)) {
               count += 1;
            }
         }
      }
   }

   if (count == 0) return {};

   std::vector<game_mode_req_entry_edit> entries;

   for (uint32 game_mode_index = 0; game_mode_index < game_modes.size();
        ++game_mode_index) {
      std::span<const world::requirement_list> requirements =
         game_modes[game_mode_index].requirements;

      for (uint32 list_index = 0; list_index < requirements.size(); ++list_index) {
         const world::requirement_list& req_list = requirements[list_index];

         if (not string::iequals("world", req_list.file_type)) {
            continue;
         }

         for (uint32 entry_index = 0; entry_index < req_list.entries.size();
              ++entry_index) {
            if (string::iequals(req_list.entries[entry_index], old_value)) {
               entries.push_back(
                  game_mode_req_entry_edit{.game_mode_index = game_mode_index,
                                           .list_index = list_index,
                                           .entry_index = entry_index,
                                           .value = std::string{new_value}});
            }
         }
      }
   }

   return entries;
}

struct rename_layer final : edit<world::edit_context> {
   rename_layer(uint32 index, std::string new_name,
                std::optional<req_entry_edit> req_entry,
                std::vector<game_mode_req_entry_edit> game_mode_req_entries)
      : index{index},
        name{std::move(new_name)},
        req_entry{std::move(req_entry)},
        game_mode_req_entries{std::move(game_mode_req_entries)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      apply_common(context);

      context.world.deleted_layers.push_back(name);
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply_common(context);

      context.world.deleted_layers.pop_back();
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const rename_layer* other = dynamic_cast<const rename_layer*>(&other_unknown);

      if (not other) return false;

      if (this->index != other->index or
          this->req_entry.has_value() != other->req_entry.has_value() or
          this->game_mode_req_entries.size() != other->game_mode_req_entries.size()) {
         return false;
      }

      if (this->req_entry->list_index != other->req_entry->list_index or
          this->req_entry->entry_index != other->req_entry->entry_index) {
         return false;
      }

      for (std::size_t i = 0; i < game_mode_req_entries.size(); ++i) {
         if (this->game_mode_req_entries[i].game_mode_index !=
                other->game_mode_req_entries[i].game_mode_index or
             this->game_mode_req_entries[i].list_index !=
                other->game_mode_req_entries[i].list_index or
             this->game_mode_req_entries[i].entry_index !=
                other->game_mode_req_entries[i].entry_index) {
            return false;
         }
      }

      return true;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      rename_layer& other = dynamic_cast<rename_layer&>(other_unknown);

      name = std::move(other.name);
      req_entry = std::move(other.req_entry);
      game_mode_req_entries = std::move(other.game_mode_req_entries);
   }

private:
   void apply_common(world::edit_context& context) noexcept
   {
      assert(index < context.world.layer_descriptions.size());

      world::world& world = context.world;

      std::swap(world.layer_descriptions[index].name, name);

      if (req_entry) {
         assert(req_entry->list_index < world.requirements.size());
         assert(req_entry->entry_index <
                world.requirements[req_entry->list_index].entries.size());

         std::swap(world.requirements[req_entry->list_index].entries[req_entry->entry_index],
                   req_entry->value);
      }

      for (game_mode_req_entry_edit& entry : game_mode_req_entries) {
         assert(entry.game_mode_index < world.game_modes.size());
         assert(entry.list_index <
                world.game_modes[entry.game_mode_index].requirements.size());
         assert(entry.entry_index < world.game_modes[entry.game_mode_index]
                                       .requirements[entry.list_index]
                                       .entries.size());

         std::swap(world.game_modes[entry.game_mode_index]
                      .requirements[entry.list_index]
                      .entries[entry.entry_index],
                   entry.value);
      }
   }

   uint32 index = 0;
   std::string name;

   std::optional<req_entry_edit> req_entry;
   std::vector<game_mode_req_entry_edit> game_mode_req_entries;
};

}

auto make_rename_layer(uint32 index, std::string new_name, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   assert(index < world.layer_descriptions.size());

   const std::string new_req_name = fmt::format("{}_{}", world.name, new_name);
   const std::string old_req_name =
      fmt::format("{}_{}", world.name, world.layer_descriptions[index].name);

   return std::make_unique<rename_layer>(
      index, std::move(new_name),
      make_req_entry_edit(new_req_name, old_req_name, world.requirements),
      make_game_mode_req_entry_edits(new_req_name, old_req_name, world.game_modes));
}

}
