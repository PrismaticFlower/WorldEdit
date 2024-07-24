#include "tool_visualizers.hpp"

namespace we::world {

tool_visualizers::tool_visualizers() noexcept
{
   _lines_overlay.reserve(256);
   _lines.reserve(256);
   _octahedrons.reserve(64);
   _octahedrons_wireframe.reserve(64);
   _arrows_wireframe.reserve(64);
   _ghost_objects.reserve(64);
   _hub_highlights.reserve(8);
   _connection_highlights.reserve(8);
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

void tool_visualizers::add_octahedron_wireframe(float4x4 transform, float3 color)
{
   _octahedrons_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void tool_visualizers::add_arrow_wireframe(float4x4 transform, float4 color)
{
   _arrows_wireframe.emplace_back(transform, color);
}

void tool_visualizers::add_ghost_object(float4x4 transform, id<object> object_id)
{
   _ghost_objects.emplace_back(object_id, transform);
}

void tool_visualizers::add_highlight(id<planning_hub> hub_id, float3 color)
{
   _hub_highlights.emplace_back(hub_id, color);
}

void tool_visualizers::add_highlight(id<planning_connection> connection_id, float3 color)
{
   _connection_highlights.emplace_back(connection_id, color);
}

void tool_visualizers::clear() noexcept
{
   _lines_overlay.clear();
   _lines.clear();
   _octahedrons.clear();
   _octahedrons_wireframe.clear();
   _arrows_wireframe.clear();
   _ghost_objects.clear();
   _hub_highlights.clear();
   _connection_highlights.clear();
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

auto tool_visualizers::octahedrons_wireframe() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _octahedrons_wireframe;
}

auto tool_visualizers::arrows_wireframe() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _arrows_wireframe;
}

auto tool_visualizers::ghost_objects() const noexcept
   -> std::span<const tool_visualizers_ghost>
{
   return _ghost_objects;
}

auto tool_visualizers::hub_highlights() const noexcept
   -> std::span<const tool_visualizers_hub_highlight>
{
   return _hub_highlights;
}

auto tool_visualizers::connection_highlights() const noexcept
   -> std::span<const tool_visualizers_connection_highlight>
{
   return _connection_highlights;
}

}