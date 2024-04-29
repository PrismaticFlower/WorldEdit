#include "tool_visualizers.hpp"

namespace we::world {

tool_visualizers::tool_visualizers() noexcept
{
   _lines_overlay.reserve(256);
   _lines.reserve(256);
   _octahedrons.reserve(64);
   _arrows_wireframe.reserve(64);
}

void tool_visualizers::add_line_overlay(float3 v0, float3 v1, uint32 color)
{
   _lines_overlay.emplace_back(v0, color, v1, color);
}

void tool_visualizers::add_line_overlay(float3 v0, uint32 v0_color, float3 v1,
                                        uint32 v1_color)
{
   _lines_overlay.emplace_back(v0, v0_color, v1, v1_color);
}

void tool_visualizers::add_line(float3 v0, float3 v1, uint32 color)
{
   _lines.emplace_back(v0, color, v1, color);
}

void tool_visualizers::add_line(float3 v0, uint32 v0_color, float3 v1, uint32 v1_color)
{
   _lines.emplace_back(v0, v0_color, v1, v1_color);
}

void tool_visualizers::add_octahedron(float4x4 transform, float4 color)
{
   _octahedrons.emplace_back(transform, color);
}

void tool_visualizers::add_arrow_wireframe(float4x4 transform, float4 color)
{
   _arrows_wireframe.emplace_back(transform, color);
}

void tool_visualizers::clear() noexcept
{
   _lines_overlay.clear();
   _lines.clear();
   _octahedrons.clear();
   _arrows_wireframe.clear();
}

auto tool_visualizers::lines_overlay() const noexcept
   -> std::span<const tool_visualizers_line>
{
   return _lines_overlay;
}

auto tool_visualizers::lines() const noexcept -> std::span<const tool_visualizers_line>
{
   return _lines;
}

auto tool_visualizers::octahedrons() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _octahedrons;
}

auto tool_visualizers::arrows_wireframe() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _arrows_wireframe;
}

}