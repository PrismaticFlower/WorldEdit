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

auto make_req_entry_edit(std::string_view new_value, std::string_view old_value,
                         std::span<const world::requirement_list> req_list) noexcept
   -> std::optional<req_entry_edit>
{
   for (uint32 list_index = 0; list_index < req_list.size(); ++list_index) {
      for (uint32 entry_index = 0;
           entry_index < req_list[list_index].entries.size(); ++entry_index) {
         if (not string::iequals("lvl", req_list[list_index].file_type)) {
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

struct rename_game_mode final : edit<world::edit_context> {
   rename_game_mode(uint32 index, std::string new_name,
                    std::optional<req_entry_edit> req_entry)
      : index{index}, name{std::move(new_name)}, req_entry{std::move(req_entry)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      apply_common(context);

      context.world.deleted_game_modes.push_back(name);
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply_common(context);

      context.world.deleted_game_modes.pop_back();
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const rename_game_mode* other =
         dynamic_cast<const rename_game_mode*>(&other_unknown);

      if (not other) return false;

      if (this->req_entry and other->req_entry) {
         if (this->req_entry->list_index != other->req_entry->list_index or
             this->req_entry->entry_index != other->req_entry->entry_index) {
            return false;
         }
      }

      if (this->req_entry->list_index != other->req_entry->list_index or
          this->req_entry->entry_index != other->req_entry->entry_index) {
         return false;
      }

      return true;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      rename_game_mode& other = dynamic_cast<rename_game_mode&>(other_unknown);

      name = std::move(other.name);
      req_entry = std::move(other.req_entry);
   }

private:
   void apply_common(world::edit_context& context) noexcept
   {
      assert(index < context.world.game_modes.size());

      world::world& world = context.world;

      std::swap(world.game_modes[index].name, name);

      if (req_entry) {
         assert(req_entry->list_index < world.requirements.size());
         assert(req_entry->entry_index <
                world.requirements[req_entry->list_index].entries.size());

         std::swap(world.requirements[req_entry->list_index].entries[req_entry->entry_index],
                   req_entry->value);
      }
   }

   uint32 index = 0;
   std::string name;

   std::optional<req_entry_edit> req_entry;
};

}

auto make_rename_game_mode(uint32 index, std::string new_name, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   assert(index < world.game_modes.size());

   const std::string new_req_name = fmt::format("{}_{}", world.name, new_name);
   const std::string old_req_name =
      fmt::format("{}_{}", world.name, world.game_modes[index].name);

   return std::make_unique<rename_game_mode>(index, std::move(new_name),
                                             make_req_entry_edit(new_req_name, old_req_name,
                                                                 world.requirements));
}

}
