#include "add_sun_flare.hpp"

namespace we::edits {

namespace {

struct add_sun_flare final : edit<world::edit_context> {
   add_sun_flare(const world::sun_flare& sun_flare) : _sun_flare{sun_flare} {}

   void apply(world::edit_context& context) noexcept override
   {
      context.world.effects.sun_flares.push_back(_sun_flare);
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.effects.sun_flares.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::sun_flare _sun_flare;
};

}

auto make_add_sun_flare(const world::sun_flare& sun_flare)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_sun_flare>(sun_flare);
}

}
