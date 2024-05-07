#include "add_animation_group.hpp"

namespace we::edits {

namespace {

struct add_animation_group final : edit<world::edit_context> {
   add_animation_group(world::animation_group group) : group{std::move(group)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.animation_groups.push_back(std::move(group));
   }

   void revert(world::edit_context& context) noexcept override
   {
      std::swap(group, context.world.animation_groups.back());

      context.world.animation_groups.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::animation_group group;
};

}

auto make_add_animation_group(world::animation_group group)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_animation_group>(std::move(group));
}

}
