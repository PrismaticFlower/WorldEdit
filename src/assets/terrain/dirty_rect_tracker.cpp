#include "dirty_rect_tracker.hpp"

#include <utility>

namespace we::assets::terrain {

namespace {

bool overlaps(const dirty_rect& l, const dirty_rect& r) noexcept
{
   const int32 l_min_x = l.x;
   const int32 l_max_x = l.x + l.width;
   const int32 l_min_y = l.y;
   const int32 l_max_y = l.y + l.height;

   const int32 r_min_x = r.x;
   const int32 r_max_x = r.x + r.width;
   const int32 r_min_y = r.y;
   const int32 r_max_y = r.y + r.height;

   if (l_min_x >= r_min_x and l_min_x <= r_max_x) return true;
   if (l_max_x >= r_min_x and l_max_x <= r_max_x) return true;
   if (l_min_y >= r_min_y and l_min_y <= r_max_y) return true;
   if (l_max_y >= r_min_y and l_max_y <= r_max_y) return true;

   return false;
}

}

auto combine(const dirty_rect& l, const dirty_rect& r) noexcept -> dirty_rect
{
   const int32 min_x = std::min(l.x, r.x);
   const int32 min_y = std::min(l.y, r.y);
   const int32 max_x = std::max(l.x + l.width, r.x + r.width);
   const int32 max_y = std::max(l.y + l.height, r.y + r.height);

   return {.x = min_x, .y = min_y, .width = max_x - min_x, .height = max_y - min_y};
}

void dirty_rect_tracker::add(const dirty_rect rect) noexcept
{
   for (dirty_rect& other : _rects) {
      if (overlaps(rect, other)) {
         other = combine(rect, other);

         return;
      }
   }

   _rects.push_back(rect);
}

void dirty_rect_tracker::clear() noexcept
{
   _rects.clear();
}

auto dirty_rect_tracker::begin() const noexcept -> const dirty_rect*
{
   return _rects.data();
}

auto dirty_rect_tracker::end() const noexcept -> const dirty_rect*
{
   return _rects.data() + _rects.size();
}

auto dirty_rect_tracker::operator[](const std::size_t i) const noexcept
   -> const dirty_rect&
{
   return _rects[i];
}

auto dirty_rect_tracker::size() const noexcept -> std::size_t
{
   return _rects.size();
}

}