#pragma once

#include "types.hpp"

#include <span>
#include <vector>

namespace we::world {

struct tool_visualizers_line {
   float3 v0;
   uint32 v0_color;
   float3 v1;
   uint32 v1_color;
};

struct tool_visualizers {
   tool_visualizers() noexcept;

   void add_line_overlay(float3 v0, float3 v1, uint32 color);

   void add_line_overlay(float3 v0, uint32 v0_color, float3 v1, uint32 v1_color);

   void clear() noexcept;

   auto lines_overlay() const noexcept -> std::span<const tool_visualizers_line>;

private:
   std::vector<tool_visualizers_line> _lines_overlay;
};

}