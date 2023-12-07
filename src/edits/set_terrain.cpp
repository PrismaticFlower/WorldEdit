#include "set_terrain.hpp"

#include <utility>

namespace we::edits {

namespace {

struct set_terrain final : edit<world::edit_context> {
   explicit set_terrain(world::terrain terrain) : _terrain{std::move(terrain)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      std::swap(context.world.terrain, _terrain);

      const uint32 test_terrain_length =
         static_cast<uint32>(context.world.terrain.length);

      context.world.terrain.height_map_dirty.add(
         {0, 0, test_terrain_length, test_terrain_length});

      for (auto& tracker : context.world.terrain.texture_weight_maps_dirty) {
         tracker.add({0, 0, test_terrain_length, test_terrain_length});
      }

      context.world.terrain.color_map_dirty.add(
         {0, 0, test_terrain_length, test_terrain_length});

      context.world.terrain.water_map_dirty.add(
         {0, 0, test_terrain_length / 4, test_terrain_length / 4});
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::terrain _terrain;
};

}

auto make_set_terrain(world::terrain terrain)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_terrain>(std::move(terrain));
}

}