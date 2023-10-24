#pragma once

#include "types.hpp"

#include <span>
#include <vector>

namespace we::world {

struct tool_visualizers_line {
   float3 v0;
   float3 v1;
   uint32 color;
};

struct tool_visualizers {
   tool_visualizers() noexcept;

   void add_line_overlay(float3 v0, float3 v1, uint32 color);

   void clear() noexcept;

   auto lines_overlay() const noexcept -> std::span<const tool_visualizers_line>;

private:
   std::vector<tool_visualizers_line> _lines_overlay;
};

}