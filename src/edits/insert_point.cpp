#include "insert_point.hpp"
#include "world/utility/world_utilities.hpp"

namespace we::edits {

namespace {

struct insert_sector_point final : edit<world::edit_context> {
   insert_sector_point(world::sector_id sector_id,
                       std::size_t insert_before_index, float2 point)
      : _id{sector_id}, _insert_before_index{insert_before_index}, _point{point}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      auto& points = world::find_entity(context.world.sectors, _id)->points;

      points.insert(points.begin() + _insert_before_index, _point);
   }

   void revert(world::edit_context& context) noexcept override
   {
      auto& points = world::find_entity(context.world.sectors, _id)->points;

      points.erase(points.begin() + _insert_before_index);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::sector_id _id;
   const std::size_t _insert_before_index;
   const float2 _point;
};

}

auto make_insert_point(world::sector_id sector_id, std::size_t insert_before_index,
                       float2 point) -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_sector_point>(sector_id, insert_before_index, point);
}

}