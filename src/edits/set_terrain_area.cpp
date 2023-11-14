#include "set_terrain_area.hpp"

#include <cassert>
#include <utility>
#include <vector>

using namespace we::assets::terrain;

namespace we::edits {

namespace {

struct area {
   dirty_rect rect;
   container::dynamic_array_2d<int16> height_map;
};

bool is_touching(const dirty_rect& l, const dirty_rect& r) noexcept
{
   const bool x_touches = l.left <= r.right and r.left <= l.right;
   const bool y_touches = l.top <= r.bottom and r.top <= l.bottom;

   if (l.left == r.right) return y_touches;
   if (l.right == r.left) return y_touches;
   if (l.top == r.bottom) return x_touches;
   if (l.bottom == r.top) return x_touches;

   return false;
}

void copy_height_map(const dirty_rect dest_rect,
                     container::dynamic_array_2d<int16>& dst_height_map,
                     const dirty_rect src_rect,
                     const container::dynamic_array_2d<int16>& src_height_map,
                     const dirty_rect copy_rect)
{
   assert(contains(dest_rect, copy_rect) and contains(src_rect, copy_rect));

   for (uint32 y = copy_rect.top; y < copy_rect.bottom; ++y) {
      for (uint32 x = copy_rect.left; x < copy_rect.right; ++x) {
         dst_height_map[{x - dest_rect.left, y - dest_rect.top}] =
            src_height_map[{x - src_rect.left, y - src_rect.top}];
      }
   }
}

auto clip_area(const dirty_rect clipped_rect, const area& new_area) -> area
{
   assert(contains(new_area.rect, clipped_rect));

   container::dynamic_array_2d<int16> clipped_height_map{clipped_rect.right -
                                                            clipped_rect.left,
                                                         clipped_rect.bottom -
                                                            clipped_rect.top};

   for (uint32 y = clipped_rect.top; y < clipped_rect.bottom; ++y) {
      for (uint32 x = clipped_rect.left; x < clipped_rect.right; ++x) {
         clipped_height_map[{x - clipped_rect.left, y - clipped_rect.top}] =
            new_area.height_map[{x - new_area.rect.left, y - new_area.rect.top}];
      }
   }

   return {clipped_rect, std::move(clipped_height_map)};
}

struct set_terrain_area : edit<world::edit_context> {
   set_terrain_area(area area)
   {
      _areas.push_back(std::move(area));
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::terrain& terrain = context.world.terrain;

      for (area& area : _areas) {
         assert(area.rect.left < area.rect.right);
         assert(area.rect.top < area.rect.bottom);
         assert(area.rect.right <= (uint32)context.world.terrain.length);
         assert(area.rect.bottom <= (uint32)context.world.terrain.length);

         for (uint32 y = area.rect.top; y < area.rect.bottom; ++y) {
            for (uint32 x = area.rect.left; x < area.rect.right; ++x) {
               std::swap(terrain.height_map[{x, y}],
                         area.height_map[{x - area.rect.left, y - area.rect.top}]);
            }
         }

         terrain.height_map_dirty.add(area.rect);
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_terrain_area* other =
         dynamic_cast<const set_terrain_area*>(&other_unknown);

      if (not other) return false;
      if (other->_areas.size() != 1) return false;

      const dirty_rect rect = other->_areas[0].rect;

      for (const area& area : _areas) {
         if (contains(area.rect, rect)) {
            return true;
         }
         else if (contains(rect, area.rect)) {
            return true;
         }
         else if (overlaps(rect, area.rect)) {
            return true;
         }
         else if (edge_joinable(area.rect, rect)) {
            return true;
         }
         else if (is_touching(area.rect, rect)) {
            return true;
         }
      }

      return false;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      assert(is_coalescable(other_unknown));

      set_terrain_area& other = dynamic_cast<set_terrain_area&>(other_unknown);

      add(std::move(other._areas[0]));
   }

private:
   void add(area new_area) noexcept
   {
      for (std::size_t i = 0; i < _areas.size(); ++i) {
         if (contains(_areas[i].rect, new_area.rect)) {
            copy_height_map(_areas[i].rect, _areas[i].height_map, new_area.rect,
                            new_area.height_map, new_area.rect);

            return;
         }
         else if (contains(new_area.rect, _areas[i].rect)) {
            _areas.erase(_areas.begin() + i);

            return add(std::move(new_area));
         }
         else if (overlaps(new_area.rect, _areas[i].rect)) {
            copy_height_map(_areas[i].rect, _areas[i].height_map, new_area.rect,
                            new_area.height_map,
                            intersection(new_area.rect, _areas[i].rect));

            const dirty_rect existing_rect = _areas[i].rect;

            if (new_area.rect.top < existing_rect.top) {
               add(clip_area({.left = new_area.rect.left,
                              .top = new_area.rect.top,
                              .right = new_area.rect.right,
                              .bottom = existing_rect.top},
                             new_area));
            }
            if (new_area.rect.bottom > existing_rect.bottom) {
               add(clip_area({.left = new_area.rect.left,
                              .top = existing_rect.bottom,
                              .right = new_area.rect.right,
                              .bottom = new_area.rect.bottom},
                             new_area));
            }
            if (new_area.rect.left < existing_rect.left) {
               add(clip_area({.left = new_area.rect.left,
                              .top = std::max(new_area.rect.top, existing_rect.top),
                              .right = existing_rect.left,
                              .bottom = std::min(new_area.rect.bottom,
                                                 existing_rect.bottom)},
                             new_area));
            }
            if (new_area.rect.right > existing_rect.right) {
               add(clip_area({.left = existing_rect.right,
                              .top = std::max(new_area.rect.top, existing_rect.top),
                              .right = new_area.rect.right,
                              .bottom = std::min(new_area.rect.bottom,
                                                 existing_rect.bottom)},
                             new_area));
            }

            return;
         }
         else if (edge_joinable(_areas[i].rect, new_area.rect)) {
            const dirty_rect new_rect = combine(new_area.rect, _areas[i].rect);
            container::dynamic_array_2d<int16> new_height_map{new_rect.right -
                                                                 new_rect.left,
                                                              new_rect.bottom -
                                                                 new_rect.top};

            copy_height_map(new_rect, new_height_map, _areas[i].rect,
                            _areas[i].height_map, _areas[i].rect);
            copy_height_map(new_rect, new_height_map, new_area.rect,
                            new_area.height_map, new_area.rect);

            _areas.erase(_areas.begin() + i);

            return add({new_rect, std::move(new_height_map)});
         }
      }

      _areas.emplace_back(std::move(new_area));
   }

   std::vector<area> _areas;
};

}

auto make_set_terrain_area(const uint32 rect_start_x, const uint32 rect_start_y,
                           container::dynamic_array_2d<int16> rect_height_map)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_terrain_area>(
      area{.rect = {.left = rect_start_x,
                    .top = rect_start_y,
                    .right = static_cast<uint32>(rect_start_x +
                                                 rect_height_map.shape()[0]),
                    .bottom = static_cast<uint32>(rect_start_y +
                                                  rect_height_map.shape()[1])},
           .height_map = std::move(rect_height_map)});
}

}
