#include "delete_entity.hpp"
#include "types.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/world_utilities.hpp"

#include <vector>

namespace we::edits {

namespace {

struct delete_sector_point final : edit<world::edit_context> {
   delete_sector_point(world::sector_id id, uint32 point_index, float2 point)
      : _id{id}, _point_index{point_index}, _point{point}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::vector<float2>& points =
         world::find_entity(context.world.sectors, _id)->points;

      points.erase(points.begin() + _point_index);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::vector<float2>& points =
         world::find_entity(context.world.sectors, _id)->points;

      points.insert(points.begin() + _point_index, _point);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::sector_id _id;
   const uint32 _point_index;
   const float2 _point;
};

}

auto make_delete_sector_point(world::sector_id sector_id,
                              const std::size_t point_index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const world::sector* sector = world::find_entity(world.sectors, sector_id);

   return std::make_unique<delete_sector_point>(sector_id,
                                                static_cast<uint32>(point_index),
                                                sector->points[point_index]);
}

}
