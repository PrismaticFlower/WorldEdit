#include "swap_terrain_textures.hpp"

#include <utility>

namespace we::edits {

namespace {

struct swap_terrain_textures final : edit<world::edit_context> {
   explicit swap_terrain_textures(uint32 left, uint32 right)
      : _left{left}, _right{right}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::terrain& terrain = context.world.terrain;

      std::swap(terrain.texture_names[_left], terrain.texture_names[_right]);
      std::swap(terrain.texture_scales[_left], terrain.texture_scales[_right]);
      std::swap(terrain.texture_axes[_left], terrain.texture_axes[_right]);
      std::swap(terrain.texture_weight_maps[_left],
                terrain.texture_weight_maps[_right]);

      const uint32 terrain_length = static_cast<uint32>(context.world.terrain.length);

      terrain.texture_weight_maps_dirty[_left].add(
         {0, 0, terrain_length, terrain_length});
      terrain.texture_weight_maps_dirty[_right].add(
         {0, 0, terrain_length, terrain_length});
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
   uint32 _left;
   uint32 _right;
};

}

auto make_swap_terrain_textures(uint32 left, uint32 right)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<swap_terrain_textures>(left, right);
}

}