#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

namespace {

struct add_animation final : edit<world::edit_context> {
   add_animation(world::animation animation) : animation{std::move(animation)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.animations.push_back(std::move(animation));
   }

   void revert(world::edit_context& context) noexcept override
   {
      std::swap(animation, context.world.animations.back());

      context.world.animations.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::animation animation;
};

}

auto make_add_animation(world::animation animation)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_animation>(std::move(animation));
}

}
