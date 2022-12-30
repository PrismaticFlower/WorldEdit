
#include "meta_draw_batcher.hpp"

namespace we::graphics {

meta_draw_batcher::meta_draw_batcher()
{
   _octahedrons_outlined.reserve(256);
   _boxes.reserve(256);
   _spheres.reserve(256);
   _cylinders.reserve(256);
   _cones.reserve(256);
   _triangles.reserve(2048);
   _lines_solid.reserve(2048);
}

void meta_draw_batcher::clear()
{
   _octahedrons_outlined.clear();
   _boxes.clear();
   _spheres.clear();
   _cylinders.clear();
   _cones.clear();
   _triangles.clear();
   _lines_solid.clear();
}

void meta_draw_batcher::add_octahedron_outlined(const float4x4& transform,
                                                const float4& color,
                                                const float4& outline_color)
{
   _octahedrons_outlined.emplace_back(transform, color, outline_color);
}

void meta_draw_batcher::add_box(const float4x4& transform, const float4& color)
{
   _boxes.emplace_back(transform, color);
}

void meta_draw_batcher::add_sphere(const float3& position, const float radius,
                                   const float4& color)
{
   _spheres.emplace_back(position, radius, color);
}

void meta_draw_batcher::add_cylinder(const float4x4& transform, const float4& color)
{
   _cylinders.emplace_back(transform, color);
}

void meta_draw_batcher::add_cone(const float4x4& transform, const float4& color)
{
   _cones.emplace_back(transform, color);
}

void meta_draw_batcher::add_triangle(const float3& a, const float3& b,
                                     const float3& c, const uint32 color)
{
   _triangles.emplace_back(a, color);
   _triangles.emplace_back(b, color);
   _triangles.emplace_back(c, color);
}

void meta_draw_batcher::add_line_solid(const float3& a, const float3& b, const uint32 color)
{
   _lines_solid.emplace_back(a, color);
   _lines_solid.emplace_back(b, color);
}

void meta_draw_batcher::draw(gpu::graphics_command_list& command_list,
                             gpu_virtual_address frame_constant_buffer,
                             root_signature_library& root_signature_library,
                             pipeline_library& pipeline_library,
                             geometric_shapes& shapes,
                             dynamic_buffer_allocator& dynamic_buffer_allocator) const
{
   if (all_empty()) return;

   command_list.set_graphics_root_signature(root_signature_library.meta_draw.get());
   command_list.set_graphics_cbv(rs::meta_draw::frame_cbv, frame_constant_buffer);

   const auto draw_shapes = [&]<typename T>(const std::vector<T>& instances,
                                            gpu::pipeline_handle pipeline,
                                            const geometric_shape& shape) {
      auto instances_allocation =
         dynamic_buffer_allocator.allocate(sizeof(T) * instances.size());

      std::memcpy(instances_allocation.cpu_address, instances.data(),
                  sizeof(T) * instances.size());

      command_list.set_graphics_srv(rs::meta_draw::instance_data_srv,
                                    instances_allocation.gpu_address);

      command_list.set_pipeline_state(pipeline);

      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);
      command_list.ia_set_index_buffer(shape.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);

      command_list.draw_indexed_instanced(shape.index_count,
                                          static_cast<uint32>(instances.size()),
                                          0, 0, 0);
   };

   const auto draw_vertices = [&](const std::vector<meta_draw_vertex>& vertices,
                                  gpu::pipeline_handle pipeline,
                                  gpu::primitive_topology topology) {
      auto vertices_allocation = dynamic_buffer_allocator.allocate(
         sizeof(meta_draw_vertex) * vertices.size());

      std::memcpy(vertices_allocation.cpu_address, vertices.data(),
                  sizeof(meta_draw_vertex) * vertices.size());

      command_list.set_pipeline_state(pipeline);

      command_list.ia_set_primitive_topology(topology);
      command_list.ia_set_vertex_buffers(
         0, gpu::vertex_buffer_view{.buffer_location = vertices_allocation.gpu_address,
                                    .size_in_bytes = static_cast<uint32>(
                                       sizeof(meta_draw_vertex) * vertices.size()),
                                    .stride_in_bytes = sizeof(meta_draw_vertex)});

      command_list.draw_instanced(static_cast<uint32>(vertices.size()), 1, 0, 0);
   };

   if (not _octahedrons_outlined.empty()) {
      draw_shapes(_octahedrons_outlined,
                  pipeline_library.meta_draw_shape_outlined.get(),
                  shapes.octahedron());
   }

   if (not _lines_solid.empty()) {
      draw_vertices(_lines_solid, pipeline_library.meta_draw_line_solid.get(),
                    gpu::primitive_topology::linelist);
   }

   if (not _boxes.empty()) {
      draw_shapes(_boxes, pipeline_library.meta_draw_shape.get(), shapes.cube());
   }

   if (not _spheres.empty()) {
      draw_shapes(_spheres, pipeline_library.meta_draw_sphere.get(),
                  shapes.icosphere());
   }

   if (not _cylinders.empty()) {
      draw_shapes(_cylinders, pipeline_library.meta_draw_shape.get(),
                  shapes.cylinder());
   }

   if (not _cones.empty()) {
      draw_shapes(_cones, pipeline_library.meta_draw_shape.get(), shapes.cone());
   }

   if (not _triangles.empty()) {
      draw_vertices(_triangles, pipeline_library.meta_draw_triangle.get(),
                    gpu::primitive_topology::trianglelist);
   }
}

bool meta_draw_batcher::all_empty() const noexcept
{
   return _octahedrons_outlined.empty() and _boxes.empty() and
          _spheres.empty() and _cylinders.empty() and _cones.empty() and
          _triangles.empty() and _lines_solid.empty();
}

}