#include "delete_animation_group.hpp"

namespace we::edits {

namespace {

struct delete_animation_group final : edit<world::edit_context> {
   delete_animation_group(uint32 index) : index{index} {}

   void apply(world::edit_context& context) noexcept override
   {
      assert(index < context.world.animation_groups.size());

      std::swap(context.world.animation_groups[index], group);

      context.world.animation_groups.erase(context.world.animation_groups.begin() + index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(index <= context.world.animation_groups.size());

      context.world.animation_groups.insert(context.world.animation_groups.begin() + index,
                                            std::move(group));
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   uint32 index;
   world::animation_group group;
};

}

auto make_delete_animation_group(uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_animation_group>(index);
}

}
