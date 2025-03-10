#include "dirty_range_tracker.hpp"

#include <cassert>
#include <optional>

namespace we::world {

namespace {

bool contains(const blocks_dirty_range& l, const blocks_dirty_range& r) noexcept
{
   return l.begin <= r.begin and l.end > r.begin and l.end >= r.end;
}

bool joinable(const blocks_dirty_range& l, const blocks_dirty_range& r) noexcept
{
   return (l.begin >= r.begin and l.begin <= r.end) or
          (r.begin >= l.begin and r.begin <= l.end);
}

bool is_valid(const blocks_dirty_range range) noexcept
{
   return range.begin < range.end;
}

}

void blocks_dirty_range_tracker::add(const blocks_dirty_range range) noexcept
{
   assert(is_valid(range));

   for (std::size_t i = 0; i < _ranges.size(); ++i) {
      const blocks_dirty_range other = _ranges[i];

      if (contains(other, range)) {
         return;
      }
      else if (contains(range, other)) {
         _ranges.erase(_ranges.begin() + i);

         return add(range);
      }
      else if (joinable(other, range)) {
         const blocks_dirty_range combined_range = {
            .begin = std::min(range.begin, other.begin),
            .end = std::max(range.end, other.end),
         };

         _ranges.erase(_ranges.begin() + i);

         return add(combined_range);
      }
   }

   _ranges.push_back(range);
}

void blocks_dirty_range_tracker::insert_index(uint32 index) noexcept
{
   for (blocks_dirty_range& range : _ranges) {
      if (range.begin >= index) range.begin += 1;
      if (range.end > index) range.end += 1;
   }
}

void blocks_dirty_range_tracker::remove_index(uint32 index) noexcept
{
   for (auto range = _ranges.begin(); range != _ranges.end();) {
      if (range->begin >= index) range->begin -= 1;
      if (range->end >= index) range->end -= 1;

      if (range->begin == range->end or range->begin == UINT32_MAX) {
         range = _ranges.erase(range);
      }
      else {
         range += 1;
      }
   }
}

void blocks_dirty_range_tracker::clear() noexcept
{
   _ranges.clear();
}

auto blocks_dirty_range_tracker::begin() const noexcept -> const blocks_dirty_range*
{
   return _ranges.data();
}

auto blocks_dirty_range_tracker::end() const noexcept -> const blocks_dirty_range*
{
   return _ranges.data() + _ranges.size();
}

auto blocks_dirty_range_tracker::operator[](const std::size_t i) const noexcept
   -> const blocks_dirty_range&
{
   return _ranges[i];
}

auto blocks_dirty_range_tracker::size() const noexcept -> std::size_t
{
   return _ranges.size();
}

}