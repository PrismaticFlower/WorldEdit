#include "dirty_rect_tracker.hpp"

#include <cassert>
#include <utility>

namespace we::assets::terrain {

bool is_valid(const dirty_rect& rect) noexcept
{
   return rect.left < rect.right and rect.top < rect.bottom;
}

bool overlaps(const dirty_rect& l, const dirty_rect& r) noexcept
{
   const bool x_overlaps = l.left < r.right and r.left < l.right;
   const bool y_overlaps = l.top < r.bottom and r.top < l.bottom;

   return x_overlaps and y_overlaps;
}

bool contains(const dirty_rect& l, const dirty_rect& r) noexcept
{
   const bool x_in = l.left <= r.left and l.right > r.left and l.right >= r.right;
   const bool y_in = l.top <= r.top and l.bottom > r.top and l.bottom >= r.bottom;

   return x_in and y_in;
}

bool edge_joinable(const dirty_rect& l, const dirty_rect& r) noexcept
{
   const bool x_touches = l.left <= r.right and r.left <= l.right;
   const bool y_touches = l.top <= r.bottom and r.top <= l.bottom;
   const bool touches = x_touches and y_touches;

   if (l.left == r.left and l.right == r.right and touches) return true;
   if (l.top == r.top and l.bottom == r.bottom and touches) return true;

   return false;
}

auto combine(const dirty_rect& l, const dirty_rect& r) noexcept -> dirty_rect
{
   return {.left = std::min(l.left, r.left),
           .top = std::min(l.top, r.top),
           .right = std::max(l.right, r.right),
           .bottom = std::max(l.bottom, r.bottom)};
}

auto intersection(const dirty_rect& l, const dirty_rect& r) noexcept -> dirty_rect
{
   return {.left = std::max(l.left, r.left),
           .top = std::max(l.top, r.top),
           .right = std::min(l.right, r.right),
           .bottom = std::min(l.bottom, r.bottom)};
}

void dirty_rect_tracker::add(const dirty_rect rect) noexcept
{
   assert(is_valid(rect));

   for (dirty_rect& other : _rects) {
      if (contains(other, rect)) {
         return;
      }
      else if (contains(rect, other)) {
         other = rect;

         return;
      }
      else if (overlaps(rect, other)) {
         const dirty_rect overlapped = other;

         if (rect.top < overlapped.top) {
            add({.left = rect.left,
                 .top = rect.top,
                 .right = rect.right,
                 .bottom = overlapped.top});
         }
         if (rect.bottom > overlapped.bottom) {
            add({.left = rect.left,
                 .top = overlapped.bottom,
                 .right = rect.right,
                 .bottom = rect.bottom});
         }
         if (rect.left < overlapped.left) {
            add({.left = rect.left,
                 .top = std::max(rect.top, overlapped.top),
                 .right = overlapped.left,
                 .bottom = std::min(rect.bottom, overlapped.bottom)});
         }
         if (rect.right > overlapped.right) {
            add({.left = overlapped.right,
                 .top = std::max(rect.top, overlapped.top),
                 .right = rect.right,
                 .bottom = std::min(rect.bottom, overlapped.bottom)});
         }

         return;
      }
      else if (edge_joinable(other, rect)) {
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