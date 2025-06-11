#pragma once

#include "id.hpp"
#include "types.hpp"

#include <span>
#include <vector>

namespace we::world {

struct object;
struct planning_hub;
struct planning_connection;

struct tool_visualizers_line {
   float3 v0;
   uint32 v0_color;
   float3 v1;
   uint32 v1_color;
};

struct tool_visualizers_triangle {
   float3 v0;
   float3 v1;
   float3 v2;
   uint32 color;
};

struct tool_visualizers_shape {
   float4x4 transform;
   float4 color;
};

struct tool_visualizers_ghost {
   id<object> object_id;
   float4x4 transform;
};

struct tool_visualizers_hub_highlight {
   id<planning_hub> hub_id;
   float3 color;
};

struct tool_visualizers_connection_highlight {
   id<planning_connection> connection_id;
   float3 color;
};

struct tool_visualizers_mini_grid {
   float3 positionWS;
   float size = 1.0f;
   float divisions = 1.0f;
   float line_width = 0.01f;
   float3 color;
};

struct tool_visualizers {
   tool_visualizers() noexcept;

   void add_line_overlay(float3 v0, float3 v1, uint32 color);

   void add_line_overlay(float3 v0, uint32 v0_color, float3 v1, uint32 v1_color);

   void add_line(float3 v0, float3 v1, uint32 color);

   void add_line(float3 v0, uint32 v0_color, float3 v1, uint32 v1_color);

   void add_octahedron(float4x4 transform, float4 color);

   void add_octahedron_wireframe(float4x4 transform, float3 color);

   void add_arrow_wireframe(float4x4 transform, float4 color);

   void add_ghost_object(float4x4 transform, id<object> object_id);

   void add_highlight(id<planning_hub> hub_id, float3 color);

   void add_highlight(id<planning_connection> connection_id, float3 color);

   void add_mini_grid(const tool_visualizers_mini_grid& grid);

   void add_triangle_additive(const float3& v0, const float3& v1,
                              const float3& v2, uint32 color);

   void add_box_additive(const float4x4& transform, const float4& color);

   void add_ramp_additive(const float4x4& transform, const float4& color);

   void add_cylinder_additive(const float4x4& transform, const float4& color);

   void add_cone_additive(const float4x4& transform, const float4& color);

   void add_hemisphere_additive(const float4x4& transform, const float4& color);

   void add_pyramid_additive(const float4x4& transform, const float4& color);

   void clear() noexcept;

   auto lines_overlay() const noexcept -> std::span<const tool_visualizers_line>;

   auto lines() const noexcept -> std::span<const tool_visualizers_line>;

   auto octahedrons() const noexcept -> std::span<const tool_visualizers_shape>;

   auto octahedrons_wireframe() const noexcept
      -> std::span<const tool_visualizers_shape>;

   auto arrows_wireframe() const noexcept -> std::span<const tool_visualizers_shape>;

   auto ghost_objects() const noexcept -> std::span<const tool_visualizers_ghost>;

   auto hub_highlights() const noexcept
      -> std::span<const tool_visualizers_hub_highlight>;

   auto connection_highlights() const noexcept
      -> std::span<const tool_visualizers_connection_highlight>;

   auto mini_grids() const noexcept -> std::span<const tool_visualizers_mini_grid>;

   auto triangles_additive() const noexcept
      -> std::span<const tool_visualizers_triangle>;

   auto boxes_additive() const noexcept -> std::span<const tool_visualizers_shape>;

   auto ramps_additive() const noexcept -> std::span<const tool_visualizers_shape>;

   auto cylinders_additive() const noexcept -> std::span<const tool_visualizers_shape>;

   auto cones_additive() const noexcept -> std::span<const tool_visualizers_shape>;

   auto hemispheres_additive() const noexcept
      -> std::span<const tool_visualizers_shape>;

   auto pyramids_additive() const noexcept -> std::span<const tool_visualizers_shape>;

private:
   std::vector<tool_visualizers_line> _lines_overlay;
   std::vector<tool_visualizers_line> _lines;
   std::vector<tool_visualizers_shape> _octahedrons;
   std::vector<tool_visualizers_shape> _octahedrons_wireframe;
   std::vector<tool_visualizers_shape> _arrows_wireframe;
   std::vector<tool_visualizers_ghost> _ghost_objects;
   std::vector<tool_visualizers_hub_highlight> _hub_highlights;
   std::vector<tool_visualizers_connection_highlight> _connection_highlights;
   std::vector<tool_visualizers_mini_grid> _mini_grids;
   std::vector<tool_visualizers_triangle> _triangles_additive;
   std::vector<tool_visualizers_shape> _boxes_additive;
   std::vector<tool_visualizers_shape> _ramps_additive;
   std::vector<tool_visualizers_shape> _cylinders_additive;
   std::vector<tool_visualizers_shape> _cones_additive;
   std::vector<tool_visualizers_shape> _hemispheres_additive;
   std::vector<tool_visualizers_shape> _pyramids_additive;
};

}