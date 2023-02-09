#pragma once

#include "types.hpp"

#include <vector>

namespace we::world {

struct tool_visualizers_line {
   float3 v0;
   float3 v1;
   uint32 color;
};

struct tool_visualizers {
   std::vector<tool_visualizers_line> lines;

   tool_visualizers() noexcept
   {
      lines.reserve(16);
   }

   void clear() noexcept
   {
      lines.clear();
   }
};

}