#include "dirty_rect_tracker.hpp"

#include <cassert>
#include <utility>

namespace we::assets::terrain {

namespace {

bool is_valid(const dirty_rect& rect) noexcept
{
   return rect.left < rect.right and rect.top < rect.bottom;
}

bool overlaps(const dirty_rect& l, const dirty_rect& r) noexcept
{
   if (l.left >= r.left and l.left <= r.right) return true;
   if (l.right >= r.left and l.right <= r.right) return true;
   if (l.top >= r.top and l.top <= r.bottom) return true;
   if (l.bottom >= r.top and l.bottom <= r.bottom) return true;

   return false;
}

}

auto combine(const dirty_rect& l, const dirty_rect& r) noexcept -> dirty_rect
{
   return {.left = std::min(l.left, r.left),
           .top = std::min(l.top, r.top),
           .right = std::max(l.right, r.right),
           .bottom = std::max(l.bottom, r.bottom)};
}

void dirty_rect_tracker::add(const dirty_rect rect) noexcept
{
   assert(is_valid(rect));

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