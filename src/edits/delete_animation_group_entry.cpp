#include "delete_animation_group_entry.hpp"

namespace we::edits {

namespace {

struct delete_animation_group_entry_key final : edit<world::edit_context> {
   delete_animation_group_entry_key(std::vector<world::animation_group::entry>* entries,
                                    uint32 index)
      : entries{entries}, index{index}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(entries));
      assert(index < entries->size());

      std::swap((*entries)[index], entry);

      entries->erase(entries->begin() + index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(entries));
      assert(index <= entries->size());

      entries->insert(entries->begin() + index, std::move(entry));
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<world::animation_group::entry>* entries;
   uint32 index;
   world::animation_group::entry entry;
};

}

auto make_delete_animation_group_entry(std::vector<world::animation_group::entry>* entries,
                                       uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_animation_group_entry_key>(entries, index);
}

}
