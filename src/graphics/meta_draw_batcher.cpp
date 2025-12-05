
#include "meta_draw_batcher.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::graphics {

namespace {

constexpr std::array<std::array<float3, 2>, 18> arrow_outline = [] {
   constexpr std::array<std::array<uint16, 2>, 18> arrow_indices{{{2, 1},
                                                                  {4, 3},
                                                                  {4, 5},
                                                                  {3, 5},
                                                                  {1, 5},
                                                                  {2, 5},
                                                                  {8, 6},
                                                                  {6, 7},
                                                                  {7, 9},
                                                                  {10, 12},
                                                                  {13, 11},
                                                                  {11, 10},
                                                                  {6, 10},
                                                                  {11, 7},
                                                                  {2, 9},
                                                                  {1, 8},
                                                                  {4, 13},
                                                                  {3, 12}}};

   constexpr std::array<float3, 15> arrow_vertices{
      {{0.0f, 0.0f, 0.0f},
       {0.611469f, -0.366881f, 0.085396f},
       {0.611469f, 0.366881f, 0.085396f},
       {-0.611469f, -0.366881f, 0.085396f},
       {-0.611469f, 0.366881f, 0.085396f},
       {0.000000f, 0.000000f, 1.002599f},
       {0.305735f, -0.366881f, -0.984675f},
       {0.305735f, 0.366881f, -0.984675f},
       {0.305735f, -0.366881f, 0.085396f},
       {0.305735f, 0.366881f, 0.085396f},
       {-0.305735f, -0.366881f, -0.984675f},
       {-0.305735f, 0.366881f, -0.984675f},
       {-0.305735f, -0.366881f, 0.085396f},
       {-0.305735f, 0.366881f, 0.085396f},
       {-0.305735f, 0.366881f, 0.085396f}}};

   std::array<std::array<float3, 2>, 18> arrow;

   for (std::size_t i = 0; i < arrow.size(); ++i) {
      arrow[i] = {arrow_vertices[arrow_indices[i][0]],
                  arrow_vertices[arrow_indices[i][1]]};
   }

   return arrow;
}();

constexpr std::array<float2, 8> circle_outline_8 = {{
   {1.0f, 0.0f},
   {0.70710677f, 0.70710677f},
   {0.0f, 1.0f},
   {-0.70710677f, 0.70710677f},
   {-1.0f, 0.0f},
   {-0.70710665f, -0.7071069f},
   {0.0f, -1.0f},
   {0.707107f, -0.70710653f},
}};

constexpr std::array<float2, 64> circle_outline_64 = {{
   {1.0f, 0.0f},
   {0.9951847f, 0.09801714f},
   {0.98078525f, 0.19509032f},
   {0.95694035f, 0.29028466f},
   {0.9238795f, 0.38268346f},
   {0.88192123f, 0.47139674f},
   {0.8314696f, 0.55557024f},
   {0.77301043f, 0.63439333f},
   {0.70710677f, 0.70710677f},
   {0.6343933f, 0.77301043f},
   {0.5555702f, 0.83146966f},
   {0.47139665f, 0.8819213f},
   {0.38268343f, 0.9238795f},
   {0.29028463f, 0.95694035f},
   {0.19509023f, 0.9807853f},
   {0.098017134f, 0.9951847f},
   {0.0f, 1.0f},
   {-0.09801722f, 0.9951847f},
   {-0.19509032f, 0.98078525f},
   {-0.29028472f, 0.9569403f},
   {-0.38268352f, 0.9238795f},
   {-0.47139683f, 0.88192123f},
   {-0.55557036f, 0.83146954f},
   {-0.6343933f, 0.7730105f},
   {-0.70710677f, 0.70710677f},
   {-0.7730105f, 0.6343933f},
   {-0.83146966f, 0.5555702f},
   {-0.88192135f, 0.47139663f},
   {-0.9238796f, 0.38268328f},
   {-0.95694035f, 0.29028472f},
   {-0.9807853f, 0.19509031f},
   {-0.9951847f, 0.0980171f},
   {-1.0f, 0.0f},
   {-0.9951847f, -0.09801727f},
   {-0.98078525f, -0.19509049f},
   {-0.9569403f, -0.29028487f},
   {-0.9238795f, -0.38268343f},
   {-0.88192123f, -0.47139677f},
   {-0.83146954f, -0.5555703f},
   {-0.7730104f, -0.6343934f},
   {-0.70710665f, -0.7071069f},
   {-0.63439333f, -0.77301043f},
   {-0.55557f, -0.8314698f},
   {-0.47139668f, -0.8819213f},
   {-0.38268313f, -0.9238797f},
   {-0.29028454f, -0.95694035f},
   {-0.19509038f, -0.98078525f},
   {-0.09801693f, -0.9951847f},
   {0.0f, -1.0f},
   {0.09801743f, -0.9951847f},
   {0.19509041f, -0.98078525f},
   {0.29028502f, -0.95694023f},
   {0.3826836f, -0.92387944f},
   {0.4713967f, -0.8819213f},
   {0.5555704f, -0.8314695f},
   {0.63439333f, -0.77301043f},
   {0.707107f, -0.70710653f},
   {0.77301055f, -0.63439316f},
   {0.8314696f, -0.5555703f},
   {0.88192135f, -0.47139654f},
   {0.92387956f, -0.38268343f},
   {0.9569404f, -0.2902844f},
   {0.9807853f, -0.19509023f},
   {0.9951848f, -0.09801677f},
}};

constexpr std::size_t line_max_batch_size = 16384;

}

meta_draw_batcher::meta_draw_batcher()
{
   _octahedrons_outlined.reserve(256);

   _octahedrons.reserve(256);
   _boxes.reserve(256);
   _spheres.reserve(256);
   _cylinders.reserve(256);
   _cones.reserve(256);
   _ramps.reserve(8);
   _hemispheres.reserve(8);
   _pyramids.reserve(8);
   _triangles.reserve(2048);
   _lines_solid.reserve(2048);

   _octahedrons_wireframe.reserve(32);
   _boxes_wireframe.reserve(16);
   _spheres_wireframe.reserve(16);
   _cylinders_wireframe.reserve(16);
   _cones_wireframe.reserve(16);
   _ramps_wireframe.reserve(16);
   _hemispheres_wireframe.reserve(8);
   _pyramids_wireframe.reserve(8);
   _triangles_wireframe.reserve(2048);

   _lines_overlay.reserve(2048);
}

void meta_draw_batcher::clear()
{
   _octahedrons_outlined.clear();

   _octahedrons.clear();
   _hint_hexahedrons.clear();
   _boxes.clear();
   _spheres.clear();
   _cylinders.clear();
   _cones.clear();
   _ramps.clear();
   _hemispheres.clear();
   _pyramids.clear();
   _triangles.clear();
   _lines_solid.clear();

   _octahedrons_wireframe.clear();
   _hint_hexahedrons_wireframe.clear();
   _boxes_wireframe.clear();
   _spheres_wireframe.clear();
   _cylinders_wireframe.clear();
   _cones_wireframe.clear();
   _ramps_wireframe.clear();
   _hemispheres_wireframe.clear();
   _pyramids_wireframe.clear();
   _triangles_wireframe.clear();

   _lines_overlay.clear();
}

void meta_draw_batcher::add_octahedron_outlined(const float4x4& transform,
                                                const float4& color,
                                                const float4& outline_color)
{
   _octahedrons_outlined.emplace_back(transform, color, outline_color);
}

void meta_draw_batcher::add_octahedron(const float4x4& transform, const float4& color)
{
   _octahedrons.emplace_back(transform, color);
}

void meta_draw_batcher::add_hint_hexahedron(const float4x4& transform, const float4& color)
{
   _hint_hexahedrons.emplace_back(transform, color);
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

void meta_draw_batcher::add_ramp(const float4x4& transform, const float4& color)
{
   _ramps.emplace_back(transform, color);
}

void meta_draw_batcher::add_hemisphere(const float4x4& transform, const float4& color)
{
   _hemispheres.emplace_back(transform, color);
}

void meta_draw_batcher::add_pyramid(const float4x4& transform, const float4& color)
{
   _pyramids.emplace_back(transform, color);
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
   _lines_solid.emplace_back(a, color, b, color);
}

void meta_draw_batcher::add_line_solid(const float3& a, const uint32 color_a,
                                       const float3& b, const uint32 color_b)
{
   _lines_solid.emplace_back(a, color_a, b, color_b);
}

void meta_draw_batcher::add_arrow_outline_solid(const float4x4& transform,
                                                const float arrow_offset,
                                                const uint32 color)
{
   for (const auto& line : arrow_outline) {
      add_line_solid(transform * (line[0] + float3{0.0f, 0.0f, arrow_offset}),
                     transform * (line[1] + float3{0.0f, 0.0f, arrow_offset}), color);
   }
}

void meta_draw_batcher::add_box_outline_solid(const float4x4& transform, const uint32 color)
{
   std::array<float3, 8> verticesRS = {{
      {-1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, 1.0f},
      {-1.0f, 1.0f, 1.0f},

      {-1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, 1.0f},
      {-1.0f, -1.0f, 1.0f},
   }};

   std::array<float3, 8> verticesWS = {
      transform * verticesRS[0], transform * verticesRS[1],
      transform * verticesRS[2], transform * verticesRS[3],
      transform * verticesRS[4], transform * verticesRS[5],
      transform * verticesRS[6], transform * verticesRS[7],
   };

   add_line_solid(verticesWS[0], verticesWS[1], color);
   add_line_solid(verticesWS[1], verticesWS[2], color);
   add_line_solid(verticesWS[2], verticesWS[3], color);
   add_line_solid(verticesWS[3], verticesWS[0], color);

   add_line_solid(verticesWS[4], verticesWS[5], color);
   add_line_solid(verticesWS[5], verticesWS[6], color);
   add_line_solid(verticesWS[6], verticesWS[7], color);
   add_line_solid(verticesWS[7], verticesWS[4], color);

   add_line_solid(verticesWS[0], verticesWS[4], color);
   add_line_solid(verticesWS[1], verticesWS[5], color);
   add_line_solid(verticesWS[2], verticesWS[6], color);
   add_line_solid(verticesWS[3], verticesWS[7], color);
}

void meta_draw_batcher::add_sphere_outline_solid(const float3& position,
                                                 const float radius, const uint32 color)
{
   for (int j = 0; j < 4; ++j) {
      const quaternion rot = make_quat_from_euler({
         0.0f,
         j / 4.0f * 3.1415927f,
         0.0f,
      });

      float last_y = circle_outline_64[0].x * radius;
      float last_z = circle_outline_64[0].y * radius;

      for (std::size_t i = 1; i <= 64; ++i) {
         const float y = circle_outline_64[i % circle_outline_64.size()].x * radius;
         const float z = circle_outline_64[i % circle_outline_64.size()].y * radius;

         add_line_solid(rot * float3{0.0f, y, z} + position,
                        rot * float3{0.0f, last_y, last_z} + position, color);

         last_y = y;
         last_z = z;
      }
   }
}

void meta_draw_batcher::add_cylinder_outline_solid(const float4x4& transform,
                                                   const uint32 color)
{
   float last_x = circle_outline_64[0].x;
   float last_z = circle_outline_64[0].y;

   for (int i = 1; i <= 64; ++i) {
      const float x = circle_outline_64[i % circle_outline_64.size()].x;
      const float z = circle_outline_64[i % circle_outline_64.size()].y;

      add_line_solid(transform * float3{x, -1.0f, z},
                     transform * float3{last_x, -1.0f, last_z}, color);
      add_line_solid(transform * float3{x, 1.0f, z},
                     transform * float3{last_x, 1.0f, last_z}, color);

      last_x = x;
      last_z = z;
   }

   for (const float2& point : circle_outline_8) {
      const float x = point.x;
      const float z = point.y;

      add_line_solid(transform * float3{x, -1.0f, z},
                     transform * float3{x, 1.0f, z}, color);
   }
}

void meta_draw_batcher::add_octahedron_wireframe(const float4x4& transform,
                                                 const float3& color)
{
   _octahedrons_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void meta_draw_batcher::add_hint_hexahedron_wireframe(const float4x4& transform,
                                                      const float3& color)
{
   _hint_hexahedrons_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void meta_draw_batcher::add_box_wireframe(const float4x4& transform, const float3& color)
{
   _boxes_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void meta_draw_batcher::add_sphere_wireframe(const float3& position,
                                             const float radius, const float3& color)
{
   _spheres_wireframe.emplace_back(position, radius, float4{color, 1.0f});
}

void meta_draw_batcher::add_cylinder_wireframe(const float4x4& transform,
                                               const float3& color)
{
   _cylinders_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void meta_draw_batcher::add_cone_wireframe(const float4x4& transform, const float3& color)
{
   _cones_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void meta_draw_batcher::add_ramp_wireframe(const float4x4& transform, const float3& color)
{
   _ramps_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void meta_draw_batcher::add_hemisphere_wireframe(const float4x4& transform,
                                                 const float3& color)
{
   _hemispheres_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void meta_draw_batcher::add_pyramid_wireframe(const float4x4& transform,
                                              const float3& color)
{
   _pyramids_wireframe.emplace_back(transform, float4{color, 1.0f});
}

void meta_draw_batcher::add_triangle_wireframe(const float3& a, const float3& b,
                                               const float3& c, const uint32 color)
{
   _triangles_wireframe.emplace_back(a, color);
   _triangles_wireframe.emplace_back(b, color);
   _triangles_wireframe.emplace_back(c, color);
}

void meta_draw_batcher::add_line_overlay(const float3& a, const float3& b,
                                         const uint32 color)
{
   _lines_overlay.emplace_back(a, color, b, color);
}

void meta_draw_batcher::add_line_overlay(const float3& a, const uint32 color_a,
                                         const float3& b, const uint32 color_b)
{
   _lines_overlay.emplace_back(a, color_a, b, color_b);
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

   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

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

      command_list.ia_set_index_buffer(shape.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);

      command_list.draw_indexed_instanced(shape.index_count,
                                          static_cast<uint32>(instances.size()),
                                          0, 0, 0);
   };

   const auto draw_lines = [&](const std::vector<meta_draw_line>& lines,
                               gpu::pipeline_handle pipeline) {
      const std::size_t batches = lines.size() / line_max_batch_size;

      for (std::size_t i = 0; i < batches; ++i) {
         auto points_allocation = dynamic_buffer_allocator.allocate(
            sizeof(meta_draw_line) * line_max_batch_size);

         std::memcpy(points_allocation.cpu_address,
                     lines.data() + i * line_max_batch_size,
                     sizeof(meta_draw_line) * line_max_batch_size);

         command_list.set_pipeline_state(pipeline);

         command_list.set_graphics_srv(rs::meta_draw::instance_data_srv,
                                       points_allocation.gpu_address);

         command_list.draw_instanced(static_cast<uint32>(line_max_batch_size) * 6,
                                     1, 0, 0);
      }

      const std::size_t remainder_batch_size =
         lines.size() - batches * line_max_batch_size;

      if (remainder_batch_size == 0) return;

      auto points_allocation = dynamic_buffer_allocator.allocate(
         sizeof(meta_draw_line) * remainder_batch_size);

      std::memcpy(points_allocation.cpu_address,
                  lines.data() + batches * line_max_batch_size,
                  sizeof(meta_draw_line) * remainder_batch_size);

      command_list.set_pipeline_state(pipeline);

      command_list.set_graphics_srv(rs::meta_draw::instance_data_srv,
                                    points_allocation.gpu_address);

      command_list.draw_instanced(static_cast<uint32>(remainder_batch_size) * 6,
                                  1, 0, 0);
   };

   const auto draw_vertices = [&](const std::vector<meta_draw_vertex>& vertices,
                                  gpu::pipeline_handle pipeline) {
      auto vertices_allocation = dynamic_buffer_allocator.allocate(
         sizeof(meta_draw_vertex) * vertices.size());

      std::memcpy(vertices_allocation.cpu_address, vertices.data(),
                  sizeof(meta_draw_vertex) * vertices.size());

      command_list.set_pipeline_state(pipeline);

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
      draw_lines(_lines_solid, pipeline_library.meta_draw_line_solid.get());
   }

   if (not _octahedrons_wireframe.empty()) {
      draw_shapes(_octahedrons_wireframe,
                  pipeline_library.meta_draw_shape_wireframe.get(),
                  shapes.octahedron());
   }

   if (not _hint_hexahedrons_wireframe.empty()) {
      draw_shapes(_hint_hexahedrons_wireframe,
                  pipeline_library.meta_draw_shape_wireframe.get(),
                  shapes.hint_hexahedron());
   }

   if (not _boxes_wireframe.empty()) {
      draw_shapes(_boxes_wireframe,
                  pipeline_library.meta_draw_shape_wireframe.get(), shapes.cube());
   }

   if (not _spheres_wireframe.empty()) {
      draw_shapes(_spheres_wireframe,
                  pipeline_library.meta_draw_sphere_wireframe.get(),
                  shapes.icosphere());
   }

   if (not _cylinders_wireframe.empty()) {
      draw_shapes(_cylinders_wireframe,
                  pipeline_library.meta_draw_shape_wireframe.get(),
                  shapes.cylinder());
   }

   if (not _cones_wireframe.empty()) {
      draw_shapes(_cones_wireframe,
                  pipeline_library.meta_draw_shape_wireframe.get(), shapes.cone());
   }

   if (not _ramps_wireframe.empty()) {
      draw_shapes(_ramps_wireframe,
                  pipeline_library.meta_draw_shape_wireframe.get(), shapes.ramp());
   }

   if (not _hemispheres_wireframe.empty()) {
      draw_shapes(_hemispheres_wireframe,
                  pipeline_library.meta_draw_shape_wireframe.get(),
                  shapes.hemisphere());
   }

   if (not _pyramids_wireframe.empty()) {
      draw_shapes(_pyramids_wireframe,
                  pipeline_library.meta_draw_shape_wireframe.get(), shapes.pyramid());
   }

   if (not _triangles_wireframe.empty()) {
      draw_vertices(_triangles_wireframe,
                    pipeline_library.meta_draw_triangle_wireframe.get());
   }

   if (not _octahedrons.empty()) {
      draw_shapes(_octahedrons, pipeline_library.meta_draw_shape.get(),
                  shapes.octahedron());
   }

   if (not _hint_hexahedrons.empty()) {
      draw_shapes(_hint_hexahedrons, pipeline_library.meta_draw_shape.get(),
                  shapes.hint_hexahedron());
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

   if (not _ramps.empty()) {
      draw_shapes(_ramps, pipeline_library.meta_draw_shape.get(), shapes.ramp());
   }

   if (not _hemispheres.empty()) {
      draw_shapes(_hemispheres, pipeline_library.meta_draw_shape.get(),
                  shapes.hemisphere());
   }

   if (not _pyramids.empty()) {
      draw_shapes(_pyramids, pipeline_library.meta_draw_shape.get(), shapes.pyramid());
   }

   if (not _triangles.empty()) {
      draw_vertices(_triangles, pipeline_library.meta_draw_triangle.get());
   }

   if (not _lines_overlay.empty()) {
      draw_lines(_lines_overlay, pipeline_library.meta_draw_line_overlay.get());
   }
}

bool meta_draw_batcher::all_empty() const noexcept
{
   return _octahedrons_outlined.empty() and       //
          _octahedrons.empty() and                //
          _hint_hexahedrons.empty() and           //
          _boxes.empty() and                      //
          _spheres.empty() and                    //
          _cylinders.empty() and                  //
          _cones.empty() and                      //
          _hemispheres.empty() and                //
          _pyramids.empty() and                   //
          _ramps.empty() and                      //
          _triangles.empty() and                  //
          _lines_solid.empty() and                //
          _octahedrons_wireframe.empty() and      //
          _hint_hexahedrons_wireframe.empty() and //
          _boxes_wireframe.empty() and            //
          _spheres_wireframe.empty() and          //
          _cylinders_wireframe.empty() and        //
          _cones_wireframe.empty() and            //
          _ramps_wireframe.empty() and            //
          _hemispheres_wireframe.empty() and      //
          _pyramids_wireframe.empty() and         //
          _triangles_wireframe.empty() and        //
          _lines_overlay.empty();
}

}