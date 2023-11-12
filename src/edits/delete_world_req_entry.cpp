#include "delete_world_req_entry.hpp"

namespace we::edits {

namespace {

struct delete_world_req_entry final : edit<world::edit_context> {
   delete_world_req_entry(int list_index, int entry_index, std::string name)
      : _list_index{list_index}, _entry_index{entry_index}, _name{std::move(name)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      auto& entries = context.world.requirements[_list_index].entries;

      entries.erase(entries.begin() + _entry_index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      auto& entries = context.world.requirements[_list_index].entries;

      entries.insert(entries.begin() + _entry_index, _name);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const int _list_index;
   const int _entry_index;
   const std::string _name;
};

}

auto make_delete_world_req_entry(int list_index, int entry_index,
                                 const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_world_req_entry>(
      list_index, entry_index, world.requirements[list_index].entries[entry_index]);
}

}