#include "set_terrain_area.hpp"

#include <cassert>
#include <utility>
#include <vector>

using namespace we::assets::terrain;

namespace we::edits {

namespace {

struct access_height_map {
   access_height_map() = default;

   auto target_map(world::terrain& terrain) -> container::dynamic_array_2d<int16>&
   {
      return terrain.height_map;
   }

   void mark_dirty(world::terrain& terrain, dirty_rect rect)
   {
      terrain.height_map_dirty.add(rect);
   }

   bool can_coalesce(const access_height_map&) const noexcept
   {
      return true;
   }
};

struct access_texture_weight_map {
   access_texture_weight_map(uint32 index) : _index{static_cast<uint8>(index)}
   {
      assert(index < terrain::texture_count);
   };

   auto target_map(world::terrain& terrain) -> container::dynamic_array_2d<uint8>&
   {
      return terrain.texture_weight_maps[_index];
   }

   void mark_dirty(world::terrain& terrain, dirty_rect rect)
   {
      terrain.texture_weight_maps_dirty[_index].add(rect);
   }

   bool can_coalesce(const access_texture_weight_map& other) const noexcept
   {
      return this->_index == other._index;
   }

private:
   uint8 _index = 0;
};

struct access_color_map {
   access_color_map() = default;

   auto target_map(world::terrain& terrain) -> container::dynamic_array_2d<uint32>&
   {
      return terrain.color_map;
   }

   void mark_dirty(world::terrain& terrain, dirty_rect rect)
   {
      terrain.color_or_light_map_dirty.add(rect);
   }

   bool can_coalesce(const access_color_map&) const noexcept
   {
      return true;
   }
};

struct access_light_map {
   access_light_map() = default;

   auto target_map(world::terrain& terrain) -> container::dynamic_array_2d<uint32>&
   {
      return terrain.light_map;
   }

   void mark_dirty(world::terrain& terrain, dirty_rect rect)
   {
      terrain.color_or_light_map_dirty.add(rect);
   }

   bool can_coalesce(const access_light_map&) const noexcept
   {
      return true;
   }
};

struct access_water_map {
   access_water_map() = default;

   auto target_map(world::terrain& terrain) -> container::dynamic_array_2d<bool>&
   {
      return terrain.water_map;
   }

   void mark_dirty(world::terrain& terrain, dirty_rect rect)
   {
      terrain.water_map_dirty.add(rect);
   }

   bool can_coalesce(const access_water_map&) const noexcept
   {
      return true;
   }
};

struct access_foliage_map {
   access_foliage_map() = default;

   auto target_map(world::terrain& terrain)
      -> container::dynamic_array_2d<world::foliage_patch>&
   {
      return terrain.foliage_map;
   }

   void mark_dirty(world::terrain& terrain, dirty_rect rect)
   {
      terrain.foliage_map_dirty.add(rect);
   }

   bool can_coalesce(const access_foliage_map&) const noexcept
   {
      return true;
   }
};

template<typename T>
struct area {
   dirty_rect rect;
   container::dynamic_array_2d<T> map;
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

template<typename T>
void copy_map(const dirty_rect dest_rect,
              container::dynamic_array_2d<T>& dst_map, const dirty_rect src_rect,
              const container::dynamic_array_2d<T>& src_map, const dirty_rect copy_rect)
{
   assert(contains(dest_rect, copy_rect) and contains(src_rect, copy_rect));

   for (uint32 y = copy_rect.top; y < copy_rect.bottom; ++y) {
      for (uint32 x = copy_rect.left; x < copy_rect.right; ++x) {
         dst_map[{x - dest_rect.left, y - dest_rect.top}] =
            src_map[{x - src_rect.left, y - src_rect.top}];
      }
   }
}

template<typename T>
auto clip_area(const dirty_rect clipped_rect, const area<T>& new_area) -> area<T>
{
   assert(contains(new_area.rect, clipped_rect));

   container::dynamic_array_2d<T> clipped_map{clipped_rect.right - clipped_rect.left,
                                              clipped_rect.bottom - clipped_rect.top};

   for (uint32 y = clipped_rect.top; y < clipped_rect.bottom; ++y) {
      for (uint32 x = clipped_rect.left; x < clipped_rect.right; ++x) {
         clipped_map[{x - clipped_rect.left, y - clipped_rect.top}] =
            new_area.map[{x - new_area.rect.left, y - new_area.rect.top}];
      }
   }

   return {clipped_rect, std::move(clipped_map)};
}

template<typename T, typename Access>
struct set_terrain_area : edit<world::edit_context>, Access {
   template<typename... Access_args>
   set_terrain_area(area<T> area, Access_args... args) : Access{args...}
   {
      _areas.push_back(std::move(area));
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::terrain& terrain = context.world.terrain;

      for (area<T>& area : _areas) {
         container::dynamic_array_2d<T>& target_map = Access::target_map(terrain);

         assert(area.rect.left < area.rect.right);
         assert(area.rect.top < area.rect.bottom);
         assert(area.rect.right <= (uint32)target_map.width());
         assert(area.rect.bottom <= (uint32)target_map.height());

         for (uint32 y = area.rect.top; y < area.rect.bottom; ++y) {
            for (uint32 x = area.rect.left; x < area.rect.right; ++x) {
               std::swap(target_map[{x, y}],
                         area.map[{x - area.rect.left, y - area.rect.top}]);
            }
         }

         Access::mark_dirty(terrain, area.rect);
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
      if (not Access::can_coalesce(*other)) return false;

      const dirty_rect rect = other->_areas[0].rect;

      for (const area<T>& area : _areas) {
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
   void add(area<T> new_area) noexcept
   {
      for (std::size_t i = 0; i < _areas.size(); ++i) {
         if (contains(_areas[i].rect, new_area.rect)) {
            copy_map(_areas[i].rect, _areas[i].map, new_area.rect, new_area.map,
                     new_area.rect);

            return;
         }
         else if (contains(new_area.rect, _areas[i].rect)) {
            _areas.erase(_areas.begin() + i);

            return add(std::move(new_area));
         }
         else if (overlaps(new_area.rect, _areas[i].rect)) {
            copy_map(_areas[i].rect, _areas[i].map, new_area.rect, new_area.map,
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
            container::dynamic_array_2d<T> new_map{new_rect.right - new_rect.left,
                                                   new_rect.bottom - new_rect.top};

            copy_map(new_rect, new_map, _areas[i].rect, _areas[i].map, _areas[i].rect);
            copy_map(new_rect, new_map, new_area.rect, new_area.map, new_area.rect);

            _areas.erase(_areas.begin() + i);

            return add({new_rect, std::move(new_map)});
         }
      }

      _areas.emplace_back(std::move(new_area));
   }

   std::vector<area<T>> _areas;
};

}

auto make_set_terrain_area(const uint32 rect_start_x, const uint32 rect_start_y,
                           container::dynamic_array_2d<int16> rect_height_map)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_terrain_area<int16, access_height_map>>(
      area<int16>{.rect = {.left = rect_start_x,
                           .top = rect_start_y,
                           .right = static_cast<uint32>(rect_start_x +
                                                        rect_height_map.width()),
                           .bottom = static_cast<uint32>(
                              rect_start_y + rect_height_map.height())},
                  .map = std::move(rect_height_map)});
}

auto make_set_terrain_area(const uint32 rect_start_x, const uint32 rect_start_y,
                           const uint32 texture_index,
                           container::dynamic_array_2d<uint8> rect_weight_map)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_terrain_area<uint8, access_texture_weight_map>>(
      area<uint8>{.rect = {.left = rect_start_x,
                           .top = rect_start_y,
                           .right = static_cast<uint32>(rect_start_x +
                                                        rect_weight_map.width()),
                           .bottom = static_cast<uint32>(
                              rect_start_y + rect_weight_map.height())},
                  .map = std::move(rect_weight_map)},
      texture_index);
}

auto make_set_terrain_area_color_map(const uint32 rect_start_x, const uint32 rect_start_y,
                                     container::dynamic_array_2d<uint32> rect_color_map)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_terrain_area<uint32, access_color_map>>(
      area<uint32>{.rect = {.left = rect_start_x,
                            .top = rect_start_y,
                            .right = static_cast<uint32>(rect_start_x +
                                                         rect_color_map.width()),
                            .bottom = static_cast<uint32>(
                               rect_start_y + rect_color_map.height())},
                   .map = std::move(rect_color_map)});
}

auto make_set_terrain_area_light_map(const uint32 rect_start_x, const uint32 rect_start_y,
                                     container::dynamic_array_2d<uint32> rect_light_map)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_terrain_area<uint32, access_light_map>>(
      area<uint32>{.rect = {.left = rect_start_x,
                            .top = rect_start_y,
                            .right = static_cast<uint32>(rect_start_x +
                                                         rect_light_map.width()),
                            .bottom = static_cast<uint32>(
                               rect_start_y + rect_light_map.height())},
                   .map = std::move(rect_light_map)});
}

auto make_set_terrain_area_water_map(const uint32 rect_start_x, const uint32 rect_start_y,
                                     container::dynamic_array_2d<bool> rect_water_map)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_terrain_area<bool, access_water_map>>(area<bool>{
      .rect = {.left = rect_start_x,
               .top = rect_start_y,
               .right = static_cast<uint32>(rect_start_x + rect_water_map.width()),
               .bottom = static_cast<uint32>(rect_start_y + rect_water_map.height())},
      .map = std::move(rect_water_map)});
}

auto make_set_terrain_area_foliage_map(
   const uint32 rect_start_x, const uint32 rect_start_y,
   container::dynamic_array_2d<world::foliage_patch> rect_foliage_map)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_terrain_area<world::foliage_patch, access_foliage_map>>(
      area<world::foliage_patch>{.rect = {.left = rect_start_x,
                                          .top = rect_start_y,
                                          .right = static_cast<uint32>(
                                             rect_start_x + rect_foliage_map.width()),
                                          .bottom = static_cast<uint32>(
                                             rect_start_y + rect_foliage_map.height())},
                                 .map = std::move(rect_foliage_map)});
}

}
