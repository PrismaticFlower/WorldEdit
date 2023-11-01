#pragma once

#include "types.hpp"

#include <vector>

namespace we::assets::terrain {

struct dirty_rect {
   int32 x = 0;
   int32 y = 0;
   int32 width = 0;
   int32 height = 0;

   constexpr bool operator==(const dirty_rect&) const noexcept = default;
};

auto combine(const dirty_rect& l, const dirty_rect& r) noexcept -> dirty_rect;

struct dirty_rect_tracker {
   void add(const dirty_rect rect) noexcept;

   void clear() noexcept;

   auto begin() const noexcept -> const dirty_rect*;

   auto end() const noexcept -> const dirty_rect*;

   auto operator[](const std::size_t i) const noexcept -> const dirty_rect&;

   auto size() const noexcept -> std::size_t;

private:
   std::vector<dirty_rect> _rects;
};

}