#include "delete_animation_hierarchy.hpp"

namespace we::edits {

namespace {

struct delete_animation_hierarchy final : edit<world::edit_context> {
   delete_animation_hierarchy(uint32 index) : index{index} {}

   void apply(world::edit_context& context) noexcept override
   {
      assert(index < context.world.animation_hierarchies.size());

      std::swap(context.world.animation_hierarchies[index], hierarchy);

      context.world.animation_hierarchies.erase(
         context.world.animation_hierarchies.begin() + index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(index <= context.world.animation_hierarchies.size());

      context.world.animation_hierarchies
         .insert(context.world.animation_hierarchies.begin() + index,
                 std::move(hierarchy));
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   uint32 index;
   world::animation_hierarchy hierarchy;
};

}

auto make_delete_animation_hierarchy(uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_animation_hierarchy>(index);
}

}
