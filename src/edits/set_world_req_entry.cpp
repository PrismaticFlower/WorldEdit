#include "set_world_req_entry.hpp"

namespace we::edits {

namespace {

struct set_world_req_entry final : edit<world::edit_context> {
   set_world_req_entry(uint32 list_index, uint32 entry_index, std::string new_value)
      : list_index{list_index}, entry_index{entry_index}, value{std::move(new_value)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      assert(list_index < context.world.requirements.size());
      assert(entry_index < context.world.requirements[list_index].entries.size());

      std::swap(context.world.requirements[list_index].entries[entry_index], value);
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(list_index < context.world.requirements.size());
      assert(entry_index < context.world.requirements[list_index].entries.size());

      std::swap(context.world.requirements[list_index].entries[entry_index], value);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_world_req_entry* other =
         dynamic_cast<const set_world_req_entry*>(&other_unknown);

      if (not other) return false;

      return this->list_index == other->list_index and
             this->entry_index == other->entry_index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_world_req_entry& other = dynamic_cast<set_world_req_entry&>(other_unknown);

      value = std::move(other.value);
   }

private:
   const uint32 list_index = 0;
   const uint32 entry_index = 0;
   std::string value;
};

}

auto make_set_world_req_entry(uint32 list_index, uint32 entry_index, std::string value)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_world_req_entry>(list_index, entry_index,
                                                std::move(value));
}

}
