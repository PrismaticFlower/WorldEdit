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
   if (l.left == r.right) return true;
   if (l.right == r.left) return true;
   if (l.top == r.bottom) return true;
   if (l.bottom == r.top) return true;

   return false;
}

void copy_height_map(const dirty_rect dest_global_rect,
                     container::dynamic_array_2d<int16>& dst_height_map,
                     const dirty_rect src_global_rect,
                     const int16* src_height_map, const std::size_t src_pitch)
{
   assert(contains(dest_global_rect, src_global_rect));

   const uint32 x_offset = src_global_rect.left - dest_global_rect.left;
   const uint32 y_offset = src_global_rect.top - dest_global_rect.top;

   for (uint32 y = src_global_rect.top; y < src_global_rect.bottom; ++y) {
      for (uint32 x = src_global_rect.left; x < src_global_rect.right; ++x) {
         const uint32 x_local_new = x - src_global_rect.left;
         const uint32 y_local_new = y - src_global_rect.top;
         const uint32 x_local_existing = x_local_new + x_offset;
         const uint32 y_local_existing = y_local_new + y_offset;

         dst_height_map[{x_local_existing, y_local_existing}] =
            src_height_map[y_local_new * src_pitch + x_local_new];
      }
   }
}

void copy_height_map(const dirty_rect dest_global_rect,
                     container::dynamic_array_2d<int16>& dst_height_map,
                     const dirty_rect src_global_rect,
                     const container::dynamic_array_2d<int16>& src_height_map)
{
   copy_height_map(dest_global_rect, dst_height_map, src_global_rect,
                   src_height_map.data(), src_height_map.shape()[0]);
}

auto clip_area(const dirty_rect clipped_rect, const area& new_area) -> area
{
   container::dynamic_array_2d<int16> clipped_height_map{clipped_rect.right -
                                                            clipped_rect.left,
                                                         clipped_rect.bottom -
                                                            clipped_rect.top};

   const uint32 x_src_offset = (new_area.rect.right - new_area.rect.left) -
                               (clipped_rect.right - clipped_rect.left);
   const uint32 y_src_offset = (new_area.rect.bottom - new_area.rect.top) -
                               (clipped_rect.bottom - clipped_rect.top);

   for (uint32 y = 0; y < clipped_rect.bottom - clipped_rect.top; ++y) {
      for (uint32 x = 0; x < clipped_rect.right - clipped_rect.left; ++x) {
         clipped_height_map[{x, y}] =
            new_area.height_map[{x + x_src_offset, y + y_src_offset}];
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
                            new_area.height_map);

            return;
         }
         else if (contains(new_area.rect, _areas[i].rect)) {
            _areas[i] = std::move(new_area);

            return;
         }
         else if (overlaps(new_area.rect, _areas[i].rect)) {
            const dirty_rect intersection_rect =
               intersection(new_area.rect, _areas[i].rect);

            copy_height_map(_areas[i].rect, _areas[i].height_map, intersection_rect,
                            new_area.height_map.data() +
                               intersection_rect.left - new_area.rect.left,
                            new_area.height_map.shape()[0]);

            if (new_area.rect.top < _areas[i].rect.top) {
               add(clip_area({.left = new_area.rect.left,
                              .top = new_area.rect.top,
                              .right = new_area.rect.right,
                              .bottom = _areas[i].rect.top},
                             new_area));
            }
            if (new_area.rect.bottom > _areas[i].rect.bottom) {
               add(clip_area({.left = new_area.rect.left,
                              .top = _areas[i].rect.bottom,
                              .right = new_area.rect.right,
                              .bottom = new_area.rect.bottom},
                             new_area));
            }
            if (new_area.rect.left < _areas[i].rect.left) {
               add(clip_area({.left = new_area.rect.left,
                              .top = std::max(new_area.rect.top, _areas[i].rect.top),
                              .right = _areas[i].rect.left,
                              .bottom = std::min(new_area.rect.bottom,
                                                 _areas[i].rect.bottom)},
                             new_area));
            }
            if (new_area.rect.right > _areas[i].rect.right) {
               add(clip_area({.left = _areas[i].rect.right,
                              .top = std::max(new_area.rect.top, _areas[i].rect.top),
                              .right = new_area.rect.right,
                              .bottom = std::min(new_area.rect.bottom,
                                                 _areas[i].rect.bottom)},
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
                            _areas[i].height_map);
            copy_height_map(new_rect, new_height_map, new_area.rect, new_area.height_map);

            _areas[i] = {new_rect, std::move(new_height_map)};

            return;
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
