#pragma once

#include "dynamic_buffer_allocator.hpp"
#include "geometric_shapes.hpp"
#include "gpu/rhi.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "types.hpp"

#include <vector>

namespace we::graphics {

struct alignas(16) meta_draw_outlined {
   float4x4 transform;
   float4 color;
   float4 outline_color;
};

static_assert(sizeof(meta_draw_outlined) == 96);

struct alignas(16) meta_draw_object {
   float4x4 transform;

   float4 color;
};

static_assert(sizeof(meta_draw_object) == 80);

struct alignas(16) meta_draw_sphere {
   float3 position;
   float radius;

   float4 color;
};

static_assert(sizeof(meta_draw_sphere) == 32);

struct alignas(16) meta_draw_vertex {
   float3 v;
   uint32 color;
};

static_assert(sizeof(meta_draw_vertex) == 16);

struct alignas(16) meta_draw_line {
   float3 position0;
   uint32 color0;
   float3 position1;
   uint32 color1;
};

static_assert(sizeof(meta_draw_line) == 32);

struct meta_draw_batcher {
   meta_draw_batcher();

   void clear();

   void add_octahedron_outlined(const float4x4& transform, const float4& color,
                                const float4& outline_color);

   void add_octahedron(const float4x4& transform, const float4& color);

   void add_hint_hexahedron(const float4x4& transform, const float4& color);

   void add_box(const float4x4& transform, const float4& color);

   void add_sphere(const float3& position, const float radius, const float4& color);

   void add_cylinder(const float4x4& transform, const float4& color);

   void add_cone(const float4x4& transform, const float4& color);

   void add_triangle(const float3& a, const float3& b, const float3& c,
                     const uint32 color);

   void add_line_solid(const float3& a, const float3& b, const uint32 color);

   void add_arrow_outline_solid(const float4x4& transform,
                                const float arrow_offset, const uint32 color);

   void add_octahedron_wireframe(const float4x4& transform, const float3& color);

   void add_hint_hexahedron_wireframe(const float4x4& transform, const float3& color);

   void add_box_wireframe(const float4x4& transform, const float3& color);

   void add_sphere_wireframe(const float3& position, const float radius,
                             const float3& color);

   void add_cylinder_wireframe(const float4x4& transform, const float3& color);

   void add_cone_wireframe(const float4x4& transform, const float3& color);

   void add_triangle_wireframe(const float3& a, const float3& b,
                               const float3& c, const uint32 color);

   void add_line_overlay(const float3& a, const float3& b, const uint32 color);

   void add_line_overlay(const float3& a, const uint32 color_a, const float3& b,
                         const uint32 color_b);

   void draw(gpu::graphics_command_list& command_list,
             gpu_virtual_address frame_constant_buffer,
             root_signature_library& root_signature_library,
             pipeline_library& pipeline_library, geometric_shapes& shapes,
             dynamic_buffer_allocator& dynamic_buffer_allocator) const;

private:
   bool all_empty() const noexcept;

   std::vector<meta_draw_outlined> _octahedrons_outlined;

   std::vector<meta_draw_object> _octahedrons;
   std::vector<meta_draw_object> _hint_hexahedrons;
   std::vector<meta_draw_object> _boxes;
   std::vector<meta_draw_sphere> _spheres;
   std::vector<meta_draw_object> _cylinders;
   std::vector<meta_draw_object> _cones;
   std::vector<meta_draw_vertex> _triangles;
   std::vector<meta_draw_line> _lines_solid;

   std::vector<meta_draw_object> _octahedrons_wireframe;
   std::vector<meta_draw_object> _hint_hexahedrons_wireframe;
   std::vector<meta_draw_object> _boxes_wireframe;
   std::vector<meta_draw_sphere> _spheres_wireframe;
   std::vector<meta_draw_object> _cylinders_wireframe;
   std::vector<meta_draw_object> _cones_wireframe;
   std::vector<meta_draw_vertex> _triangles_wireframe;

   std::vector<meta_draw_line> _lines_overlay;
};

}