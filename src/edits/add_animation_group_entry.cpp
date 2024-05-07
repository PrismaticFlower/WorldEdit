#include "add_animation_group_entry.hpp"

namespace we::edits {

namespace {

struct add_animation_group_entry final : edit<world::edit_context> {
   add_animation_group_entry(std::vector<world::animation_group::entry>* entries,
                             world::animation_group::entry new_entry)
      : entries{entries}, new_entry{std::move(new_entry)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(entries));

      entries->push_back(std::move(new_entry));
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(entries));

      std::swap(new_entry, entries->back());

      entries->pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<world::animation_group::entry>* entries;
   world::animation_group::entry new_entry;
};

}

auto make_add_animation_group_entry(std::vector<world::animation_group::entry>* entries,
                                    world::animation_group::entry new_entry)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_animation_group_entry>(entries, std::move(new_entry));
}

}