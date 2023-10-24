#include "tool_visualizers.hpp"

namespace we::world {

tool_visualizers::tool_visualizers() noexcept
{
   _lines_overlay.reserve(256);
}

void tool_visualizers::add_line_overlay(float3 v0, float3 v1, uint32 color)
{
   _lines_overlay.emplace_back(v0, v1, color);
}

void tool_visualizers::clear() noexcept
{
   _lines_overlay.clear();
}

auto tool_visualizers::lines_overlay() const noexcept
   -> std::span<const tool_visualizers_line>
{
   return _lines_overlay;
}

}