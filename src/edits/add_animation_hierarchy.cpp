#include "add_animation_hierarchy.hpp"

namespace we::edits {

namespace {

struct add_animation_hierarchy final : edit<world::edit_context> {
   add_animation_hierarchy(world::animation_hierarchy hierarchy)
      : hierarchy{std::move(hierarchy)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.animation_hierarchies.push_back(std::move(hierarchy));
   }

   void revert(world::edit_context& context) noexcept override
   {
      std::swap(hierarchy, context.world.animation_hierarchies.back());

      context.world.animation_hierarchies.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::animation_hierarchy hierarchy;
};

}

auto make_add_animation_hierarchy(world::animation_hierarchy hierarchy)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_animation_hierarchy>(std::move(hierarchy));
}

}
