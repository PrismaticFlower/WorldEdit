#pragma once

#include "types.hpp"

#include <vector>

namespace we::world {

struct blocks_dirty_range {
   uint32 begin = 0;
   uint32 end = 0;

   bool operator==(const blocks_dirty_range&) const noexcept = default;
};

struct blocks_dirty_range_tracker {
   void add(const blocks_dirty_range range) noexcept;

   void insert_index(uint32 index) noexcept;

   void remove_index(uint32 index) noexcept;

   void clear() noexcept;

   auto begin() const noexcept -> const blocks_dirty_range*;

   auto end() const noexcept -> const blocks_dirty_range*;

   auto operator[](const std::size_t i) const noexcept -> const blocks_dirty_range&;

   auto size() const noexcept -> std::size_t;

private:
   std::vector<blocks_dirty_range> _ranges;
};

}