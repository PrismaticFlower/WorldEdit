#pragma once

#include "types.hpp"

#include <vector>

namespace we::world {

struct tool_visualizers_primitive {
   float4x4 transform;
   float3 color;
};

struct tool_visualizers_line {
   float3 v0;
   float3 v1;
   uint32 color;
};

struct tool_visualizers {
   std::vector<tool_visualizers_line> lines;
   std::vector<tool_visualizers_primitive> cylinders_overlay;
   std::vector<tool_visualizers_primitive> cones_overlay;

   tool_visualizers() noexcept
   {
      lines.reserve(16);
      cylinders_overlay.reserve(16);
      cones_overlay.reserve(16);
   }

   void clear() noexcept
   {
      lines.clear();
      cylinders_overlay.clear();
      cones_overlay.clear();
   }
};

}