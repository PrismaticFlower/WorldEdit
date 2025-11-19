#include "delete_sun_flare.hpp"

namespace we::edits {

namespace {

struct delete_sun_flare final : edit<world::edit_context> {
   delete_sun_flare(uint32 index) : index{index} {}

   void apply(world::edit_context& context) noexcept override
   {
      assert(index < context.world.effects.sun_flares.size());

      sun_flare = context.world.effects.sun_flares[index];

      context.world.effects.sun_flares.erase(
         context.world.effects.sun_flares.begin() + index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(index <= context.world.effects.sun_flares.size());

      context.world.effects.sun_flares.insert(context.world.effects.sun_flares.begin() + index,
                                              sun_flare);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   uint32 index;
   world::sun_flare sun_flare;
};

}

auto make_delete_sun_flare(uint32 index) -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_sun_flare>(index);
}

}