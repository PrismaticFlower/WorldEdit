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
   _mini_grids.reserve(8);
   _triangles_additive.reserve(64);
   _boxes_additive.reserve(8);
   _ramps_additive.reserve(8);
   _cylinders_additive.reserve(8);
   _cones_additive.reserve(8);
   _hemispheres_additive.reserve(8);
   _pyramids_additive.reserve(8);
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

void tool_visualizers::add_mini_grid(const tool_visualizers_mini_grid& grid)
{
   _mini_grids.push_back(grid);
}

void tool_visualizers::add_triangle_additive(const float3& v0, const float3& v1,
                                             const float3& v2, uint32 color)
{
   _triangles_additive.emplace_back(v0, v1, v2, color);
}

void tool_visualizers::add_box_additive(const float4x4& transform, const float4& color)
{
   _boxes_additive.emplace_back(transform, color);
}

void tool_visualizers::add_ramp_additive(const float4x4& transform, const float4& color)
{
   _ramps_additive.emplace_back(transform, color);
}

void tool_visualizers::add_cylinder_additive(const float4x4& transform,
                                             const float4& color)
{
   _cylinders_additive.emplace_back(transform, color);
}

void tool_visualizers::add_cone_additive(const float4x4& transform, const float4& color)
{
   _cones_additive.emplace_back(transform, color);
}

void tool_visualizers::add_hemisphere_additive(const float4x4& transform,
                                               const float4& color)
{
   _hemispheres_additive.emplace_back(transform, color);
}

void tool_visualizers::add_pyramid_additive(const float4x4& transform, const float4& color)
{
   _pyramids_additive.emplace_back(transform, color);
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
   _mini_grids.clear();
   _triangles_additive.clear();
   _boxes_additive.clear();
   _ramps_additive.clear();
   _cylinders_additive.clear();
   _cones_additive.clear();
   _hemispheres_additive.clear();
   _pyramids_additive.clear();
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

auto tool_visualizers::mini_grids() const noexcept
   -> std::span<const tool_visualizers_mini_grid>
{
   return _mini_grids;
}

auto tool_visualizers::triangles_additive() const noexcept
   -> std::span<const tool_visualizers_triangle>
{
   return _triangles_additive;
}

auto tool_visualizers::boxes_additive() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _boxes_additive;
}

auto tool_visualizers::ramps_additive() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _ramps_additive;
}

auto tool_visualizers::cylinders_additive() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _cylinders_additive;
}

auto tool_visualizers::cones_additive() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _cones_additive;
}

auto tool_visualizers::hemispheres_additive() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _hemispheres_additive;
}

auto tool_visualizers::pyramids_additive() const noexcept
   -> std::span<const tool_visualizers_shape>
{
   return _pyramids_additive;
}
}