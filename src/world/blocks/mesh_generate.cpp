#include "mesh_generate.hpp"

#include "math/curves.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <numbers>

namespace we::world {

namespace {

enum stairway_surface {
   stairway_surface_pos_x,
   stairway_surface_neg_x,
   stairway_surface_pos_y,
   stairway_surface_neg_y,
   stairway_surface_pos_z,
   stairway_surface_neg_z
};

enum ring_surface {
   ring_surface_pos_y,
   ring_surface_neg_y,
   ring_surface_outer,
   ring_surface_inner
};

enum beveled_box_surface {
   beveled_box_surface_x,
   beveled_box_surface_y,
   beveled_box_surface_z,
   beveled_box_surface_top_edge,
   beveled_box_surface_side_edge,
   beveled_box_surface_bottom_edge,
};

enum curve_surface {
   curve_surface_pos_x,
   curve_surface_neg_x,
   curve_surface_pos_y,
   curve_surface_neg_y,
   curve_surface_pos_z,
   curve_surface_neg_z,
};

enum cylinder_surface {
   cylinder_surface_pos_y,
   cylinder_surface_neg_y,
   cylinder_surface_wall
};

auto calculate_normal(const float3& v0, const float3& v1, const float3& v2) noexcept
   -> float3
{
   const float3 edge0 = v1 - v0;
   const float3 edge1 = v2 - v0;

   return normalize(cross(edge0, edge1));
}

auto calculate_curve_axis_normal(const float3& tangent, const float3& axis) noexcept
   -> float3
{
   const float3 binormal = cross(axis, tangent);

   return cross(tangent, binormal);
}

}

auto generate_mesh(const block_custom_mesh_description_stairway& stairway) noexcept
   -> block_custom_mesh
{
   block_custom_mesh mesh;

   const float step_height = stairway.step_height;
   const float first_step_offset = stairway.first_step_offset;

   const float steps_width = stairway.size.x;
   const float steps_height = stairway.size.y;
   const float steps_length = stairway.size.z;

   const int steps = static_cast<int>(ceilf(steps_height / step_height));
   const float adjusted_step_height = steps_height / steps;
   const float step_length = steps_length / steps;
   const float inv_steps = 1.0f / steps;

   const float half_steps_width = steps_width / 2.0f;
   const float half_steps_length = steps_length / 2.0f;

   // Visible Mesh
   {
      const std::size_t step_vertex_count = 16;
      const std::size_t step_tri_count = 8;
      const std::size_t step_quad_count = 4;

      const std::size_t bottom_iertex_count = 4;
      const std::size_t bottom_tri_count = 2;
      const std::size_t bottom_quad_count = 1;

      const std::size_t back_iertex_count = 4;
      const std::size_t back_tri_count = 2;
      const std::size_t back_quad_count = 1;

      const std::size_t vertex_count =
         step_vertex_count * steps + bottom_iertex_count + back_iertex_count;
      const std::size_t tri_count =
         step_tri_count * steps + bottom_tri_count + back_tri_count;
      const std::size_t occluder_count =
         step_quad_count * steps + bottom_quad_count + back_quad_count;

      if (mesh.vertices.capacity() != vertex_count) {
         mesh.vertices = {};
         mesh.vertices.reserve(vertex_count);
      }
      else {
         mesh.vertices.clear();
      }

      if (mesh.triangles.capacity() != tri_count) {
         mesh.triangles = {};
         mesh.triangles.reserve(tri_count);
      }
      else {
         mesh.triangles.clear();
      }

      if (mesh.occluders.capacity() != occluder_count) {
         mesh.occluders = {};
         mesh.occluders.reserve(occluder_count);
      }
      else {
         mesh.occluders.clear();
      }

      uint16 vertex_index = 0;

      for (int i = 0; i < steps; ++i) {
         float step_base = i * adjusted_step_height;
         float step_top = (i + 1) * adjusted_step_height;

         step_top += first_step_offset;
         if (i != 0) step_base += first_step_offset;

         if (i + 1 == steps) step_top = steps_height + first_step_offset;

         float step_back = i * step_length - half_steps_length;
         float step_front = (i + 1) * step_length - half_steps_length;

         if (i + 1 == steps) step_front = half_steps_length;

         const uint16 top_i0 = vertex_index++;
         const uint16 top_i1 = vertex_index++;
         const uint16 top_i2 = vertex_index++;
         const uint16 top_i3 = vertex_index++;

         const float2 texcoords_y_min = {0.0f, i * inv_steps};
         const float2 texcoords_y_max = {1.0f, (i + 1) * inv_steps};

         mesh.vertices.push_back(
            {.position =
                {
                   half_steps_width,
                   step_top,
                   step_back,
                },
             .normal = {0.0f, 1.0f, 0.0f},
             .texcoords = {texcoords_y_min.x, texcoords_y_min.y},
             .surface_index = stairway_surface_pos_y});
         mesh.vertices.push_back(
            {.position =
                {
                   -half_steps_width,
                   step_top,
                   step_back,
                },
             .normal = {0.0f, 1.0f, 0.0f},
             .texcoords = {texcoords_y_max.x, texcoords_y_min.y},
             .surface_index = stairway_surface_pos_y});
         mesh.vertices.push_back(
            {.position =
                {
                   -half_steps_width,
                   step_top,
                   step_front,
                },
             .normal = {0.0f, 1.0f, 0.0f},
             .texcoords = {texcoords_y_max.x, texcoords_y_max.y},
             .surface_index = stairway_surface_pos_y});
         mesh.vertices.push_back(
            {.position =
                {
                   half_steps_width,
                   step_top,
                   step_front,
                },
             .normal = {0.0f, 1.0f, 0.0f},
             .texcoords = {texcoords_y_min.x, texcoords_y_max.y},
             .surface_index = stairway_surface_pos_y});

         const uint16 front_i0 = vertex_index++;
         const uint16 front_i1 = vertex_index++;
         const uint16 front_i2 = vertex_index++;
         const uint16 front_i3 = vertex_index++;

         const float2 texcoords_z_min = {0.0f, step_base / steps_height};
         const float2 texcoords_z_max = {1.0f, step_top / steps_height};

         mesh.vertices.push_back(
            {.position =
                {
                   -half_steps_width,
                   step_top,
                   step_back,
                },
             .normal = {0.0f, 0.0f, -1.0f},
             .texcoords = {texcoords_z_min.x, texcoords_z_min.y},
             .surface_index = stairway_surface_neg_z});
         mesh.vertices.push_back(
            {.position =
                {
                   half_steps_width,
                   step_top,
                   step_back,
                },
             .normal = {0.0f, 0.0f, -1.0f},
             .texcoords = {texcoords_z_max.x, texcoords_z_min.y},
             .surface_index = stairway_surface_neg_z});
         mesh.vertices.push_back(
            {.position =
                {
                   half_steps_width,
                   step_base,
                   step_back,
                },
             .normal = {0.0f, 0.0f, -1.0f},
             .texcoords = {texcoords_z_max.x, texcoords_z_max.y},
             .surface_index = stairway_surface_neg_z});
         mesh.vertices.push_back(
            {.position =
                {
                   -half_steps_width,
                   step_base,
                   step_back,
                },
             .normal = {0.0f, 0.0f, -1.0f},
             .texcoords = {texcoords_z_min.x, texcoords_z_max.y},
             .surface_index = stairway_surface_neg_z});

         const uint16 side_neg_i0 = vertex_index++;
         const uint16 side_neg_i1 = vertex_index++;
         const uint16 side_neg_i2 = vertex_index++;
         const uint16 side_neg_i3 = vertex_index++;

         const float2 texcoords_x_min = {step_top / steps_height, i * inv_steps};
         const float2 texcoords_x_max = {step_base / steps_height, 1.0f};

         mesh.vertices.push_back(
            {.position =
                {
                   -half_steps_width,
                   step_top,
                   step_back,
                },
             .normal = {-1.0f, 0.0f, 0.0f},
             .texcoords = {texcoords_x_min.x, texcoords_x_min.y},
             .surface_index = stairway_surface_neg_x});
         mesh.vertices.push_back(
            {.position =
                {
                   -half_steps_width,
                   step_base,
                   step_back,
                },
             .normal = {-1.0f, 0.0f, 0.0f},
             .texcoords = {texcoords_x_max.x, texcoords_x_min.y},
             .surface_index = stairway_surface_neg_x});
         mesh.vertices.push_back(
            {.position =
                {
                   -half_steps_width,
                   step_base,
                   half_steps_length,
                },
             .normal = {-1.0f, 0.0f, 0.0f},
             .texcoords = {texcoords_x_max.x, texcoords_x_max.y},
             .surface_index = stairway_surface_neg_x});
         mesh.vertices.push_back(
            {.position =
                {
                   -half_steps_width,
                   step_top,
                   half_steps_length,
                },
             .normal = {-1.0f, 0.0f, 0.0f},
             .texcoords = {texcoords_x_min.x, texcoords_x_max.y},
             .surface_index = stairway_surface_neg_x});

         const uint16 side_pos_i0 = vertex_index++;
         const uint16 side_pos_i1 = vertex_index++;
         const uint16 side_pos_i2 = vertex_index++;
         const uint16 side_pos_i3 = vertex_index++;

         mesh.vertices.push_back(
            {.position =
                {
                   half_steps_width,
                   step_base,
                   step_back,
                },
             .normal = {1.0f, 0.0f, 0.0f},
             .texcoords = {texcoords_x_max.x, texcoords_x_min.y},
             .surface_index = stairway_surface_pos_x});
         mesh.vertices.push_back(
            {.position =
                {
                   half_steps_width,
                   step_top,
                   step_back,
                },
             .normal = {1.0f, 0.0f, 0.0f},
             .texcoords = {texcoords_x_min.x, texcoords_x_min.y},
             .surface_index = stairway_surface_pos_x});
         mesh.vertices.push_back(
            {.position =
                {
                   half_steps_width,
                   step_top,
                   half_steps_length,
                },
             .normal = {1.0f, 0.0f, 0.0f},
             .texcoords = {texcoords_x_min.x, texcoords_x_max.y},
             .surface_index = stairway_surface_pos_x});
         mesh.vertices.push_back(
            {.position =
                {
                   half_steps_width,
                   step_base,
                   half_steps_length,
                },
             .normal = {1.0f, 0.0f, 0.0f},
             .texcoords = {texcoords_x_max.x, texcoords_x_max.y},
             .surface_index = stairway_surface_pos_x});

         mesh.triangles.push_back({top_i0, top_i1, top_i2});
         mesh.triangles.push_back({top_i0, top_i2, top_i3});
         mesh.occluders.push_back({top_i0, top_i1, top_i2, top_i3});

         mesh.triangles.push_back({front_i0, front_i1, front_i2});
         mesh.triangles.push_back({front_i0, front_i2, front_i3});
         mesh.occluders.push_back({front_i0, front_i1, front_i2, front_i3});

         mesh.triangles.push_back({side_neg_i0, side_neg_i1, side_neg_i2});
         mesh.triangles.push_back({side_neg_i0, side_neg_i2, side_neg_i3});
         mesh.occluders.push_back({side_neg_i0, side_neg_i1, side_neg_i2, side_neg_i3});

         mesh.triangles.push_back({side_pos_i0, side_pos_i1, side_pos_i2});
         mesh.triangles.push_back({side_pos_i0, side_pos_i2, side_pos_i3});
         mesh.occluders.push_back({side_pos_i0, side_pos_i1, side_pos_i2, side_pos_i3});
      }

      {
         const uint16 bottom_i0 = vertex_index++;
         const uint16 bottom_i1 = vertex_index++;
         const uint16 bottom_i2 = vertex_index++;
         const uint16 bottom_i3 = vertex_index++;

         mesh.vertices.push_back({.position =
                                     {
                                        half_steps_width,
                                        0.0f,
                                        -half_steps_length,
                                     },
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoords = {0.0f, 0.0f},
                                  .surface_index = stairway_surface_neg_y});
         mesh.vertices.push_back({.position =
                                     {
                                        half_steps_width,
                                        0.0f,
                                        half_steps_length,
                                     },
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoords = {1.0f, 0.0f},
                                  .surface_index = stairway_surface_neg_y});
         mesh.vertices.push_back({.position =
                                     {
                                        -half_steps_width,
                                        0.0f,
                                        half_steps_length,
                                     },
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoords = {1.0f, 1.0f},
                                  .surface_index = stairway_surface_neg_y});
         mesh.vertices.push_back({.position =
                                     {
                                        -half_steps_width,
                                        0.0f,
                                        -half_steps_length,
                                     },
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoords = {0.0f, 1.0f},
                                  .surface_index = stairway_surface_neg_y});

         mesh.triangles.push_back({bottom_i0, bottom_i1, bottom_i2});
         mesh.triangles.push_back({bottom_i0, bottom_i2, bottom_i3});
         mesh.occluders.push_back({bottom_i0, bottom_i1, bottom_i2, bottom_i3});

         const uint16 back_i0 = vertex_index++;
         const uint16 back_i1 = vertex_index++;
         const uint16 back_i2 = vertex_index++;
         const uint16 back_i3 = vertex_index++;

         mesh.vertices.push_back({.position =
                                     {
                                        half_steps_width,
                                        steps_height + first_step_offset,
                                        half_steps_length,
                                     },
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoords = {0.0f, 0.0f},
                                  .surface_index = stairway_surface_pos_z});
         mesh.vertices.push_back({.position =
                                     {
                                        -half_steps_width,
                                        steps_height + first_step_offset,
                                        half_steps_length,
                                     },
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoords = {1.0f, 0.0f},
                                  .surface_index = stairway_surface_pos_z});
         mesh.vertices.push_back({.position =
                                     {
                                        -half_steps_width,
                                        0.0f,
                                        half_steps_length,
                                     },
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoords = {1.0f, 1.0f},
                                  .surface_index = stairway_surface_pos_z});
         mesh.vertices.push_back({.position =
                                     {
                                        half_steps_width,
                                        0.0f,
                                        half_steps_length,
                                     },
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoords = {0.0f, 1.0f},
                                  .surface_index = stairway_surface_pos_z});

         mesh.triangles.push_back({back_i0, back_i1, back_i2});
         mesh.triangles.push_back({back_i0, back_i2, back_i3});
         mesh.occluders.push_back({back_i0, back_i1, back_i2, back_i3});
      }
   }

   // Collision Mesh
   {
      const std::size_t vertex_count = 42;
      const std::size_t tri_count = 20;
      const std::size_t occluder_count = 9;

      if (mesh.collision_vertices.capacity() != vertex_count) {
         mesh.collision_vertices = {};
         mesh.collision_vertices.reserve(vertex_count);
      }
      else {
         mesh.collision_vertices.clear();
      }

      if (mesh.collision_triangles.capacity() != tri_count) {
         mesh.collision_triangles = {};
         mesh.collision_triangles.reserve(tri_count);
      }
      else {
         mesh.collision_triangles.clear();
      }

      if (mesh.collision_occluders.capacity() != occluder_count) {
         mesh.collision_occluders = {};
         mesh.collision_occluders.reserve(occluder_count);
      }
      else {
         mesh.collision_occluders.clear();
      }

      uint16 vertex_index = 0;

      // Ramp Parts
      {
         const float ramp_base = first_step_offset + adjusted_step_height;
         const float ramp_top = steps_height + first_step_offset;
         const float ramp_front = (steps - 1) * step_length - half_steps_length;

         const uint16 ramp_i0 = vertex_index++;
         const uint16 ramp_i1 = vertex_index++;
         const uint16 ramp_i2 = vertex_index++;
         const uint16 ramp_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  ramp_base,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_pos_y,
         });
         mesh.collision_vertices.push_back({
            .position =
               {

                  -half_steps_width,
                  ramp_base,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_pos_y,

         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  ramp_top,
                  ramp_front,
               },
            .surface_index = stairway_surface_pos_y,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  ramp_top,
                  ramp_front,
               },
            .surface_index = stairway_surface_pos_y,
         });

         mesh.collision_triangles.push_back({ramp_i0, ramp_i1, ramp_i2});
         mesh.collision_triangles.push_back({ramp_i0, ramp_i2, ramp_i3});
         mesh.collision_occluders.push_back({ramp_i0, ramp_i1, ramp_i2, ramp_i3});

         const uint16 left_i0 = vertex_index++;
         const uint16 left_i1 = vertex_index++;
         const uint16 left_i2 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  ramp_top,
                  ramp_front,
               },
            .surface_index = stairway_surface_pos_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  ramp_base,
                  ramp_front,
               },
            .surface_index = stairway_surface_pos_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  ramp_base,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_pos_x,
         });

         mesh.collision_triangles.push_back({left_i0, left_i1, left_i2});

         const uint16 right_i0 = vertex_index++;
         const uint16 right_i1 = vertex_index++;
         const uint16 right_i2 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  ramp_base,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  ramp_base,
                  ramp_front,
               },
            .surface_index = stairway_surface_neg_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  ramp_top,
                  ramp_front,
               },
            .surface_index = stairway_surface_neg_x,
         });

         mesh.collision_triangles.push_back({right_i0, right_i1, right_i2});

         const uint16 right_front_i0 = vertex_index++;
         const uint16 right_front_i1 = vertex_index++;
         const uint16 right_front_i2 = vertex_index++;
         const uint16 right_front_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  ramp_base,
                  ramp_front,
               },
            .surface_index = stairway_surface_neg_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  ramp_base,
                  half_steps_length,
               },
            .surface_index = stairway_surface_neg_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  steps_height + first_step_offset,
                  half_steps_length,
               },
            .surface_index = stairway_surface_neg_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  steps_height + first_step_offset,
                  ramp_front,
               },
            .surface_index = stairway_surface_neg_x,
         });

         mesh.collision_triangles.push_back(
            {right_front_i0, right_front_i1, right_front_i2});
         mesh.collision_triangles.push_back(
            {right_front_i0, right_front_i2, right_front_i3});
         mesh.collision_occluders.push_back(
            {right_front_i0, right_front_i1, right_front_i2, right_front_i3});

         const uint16 left_front_i0 = vertex_index++;
         const uint16 left_front_i1 = vertex_index++;
         const uint16 left_front_i2 = vertex_index++;
         const uint16 left_front_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  steps_height + first_step_offset,
                  ramp_front,
               },
            .surface_index = stairway_surface_pos_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  steps_height + first_step_offset,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  ramp_base,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  ramp_base,
                  ramp_front,
               },
            .surface_index = stairway_surface_pos_x,
         });

         mesh.collision_triangles.push_back(
            {left_front_i0, left_front_i1, left_front_i2});
         mesh.collision_triangles.push_back(
            {left_front_i0, left_front_i2, left_front_i3});
         mesh.collision_occluders.push_back(
            {left_front_i0, left_front_i1, left_front_i2, left_front_i3});
      }

      // Top / Bottom Step Parts
      {
         const uint16 top_i0 = vertex_index++;
         const uint16 top_i1 = vertex_index++;
         const uint16 top_i2 = vertex_index++;
         const uint16 top_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  steps_height + first_step_offset,
                  (steps - 1) * step_length - half_steps_length,
               },
            .surface_index = stairway_surface_pos_y,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  steps_height + first_step_offset,
                  (steps - 1) * step_length - half_steps_length,
               },
            .surface_index = stairway_surface_pos_y,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  steps_height + first_step_offset,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_y,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  steps_height + first_step_offset,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_y,
         });

         mesh.collision_triangles.push_back({top_i0, top_i1, top_i2});
         mesh.collision_triangles.push_back({top_i0, top_i2, top_i3});
         mesh.collision_occluders.push_back({top_i0, top_i1, top_i2, top_i3});

         const uint16 bottom_i0 = vertex_index++;
         const uint16 bottom_i1 = vertex_index++;
         const uint16 bottom_i2 = vertex_index++;
         const uint16 bottom_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  0.0f,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_y,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  0.0f,
                  half_steps_length,
               },
            .surface_index = stairway_surface_neg_y,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  0.0f,
                  half_steps_length,
               },
            .surface_index = stairway_surface_neg_y,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  0.0f,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_y,
         });

         mesh.collision_triangles.push_back({bottom_i0, bottom_i1, bottom_i2});
         mesh.collision_triangles.push_back({bottom_i0, bottom_i2, bottom_i3});
         mesh.collision_occluders.push_back({bottom_i0, bottom_i1, bottom_i2, bottom_i3});

         const uint16 back_i0 = vertex_index++;
         const uint16 back_i1 = vertex_index++;
         const uint16 back_i2 = vertex_index++;
         const uint16 back_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  steps_height + first_step_offset,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_z,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  steps_height + first_step_offset,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_z,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  0.0f,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_z,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  0.0f,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_z,
         });

         mesh.collision_triangles.push_back({back_i0, back_i1, back_i2});
         mesh.collision_triangles.push_back({back_i0, back_i2, back_i3});
         mesh.collision_occluders.push_back({back_i0, back_i1, back_i2, back_i3});

         const uint16 front_i0 = vertex_index++;
         const uint16 front_i1 = vertex_index++;
         const uint16 front_i2 = vertex_index++;
         const uint16 front_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  0.0f,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_z,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  0.0f,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_z,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  adjusted_step_height + first_step_offset,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_z,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  adjusted_step_height + first_step_offset,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_z,
         });

         mesh.collision_triangles.push_back({front_i0, front_i1, front_i2});
         mesh.collision_triangles.push_back({front_i0, front_i2, front_i3});
         mesh.collision_occluders.push_back({front_i0, front_i1, front_i2, front_i3});

         const uint16 left_i0 = vertex_index++;
         const uint16 left_i1 = vertex_index++;
         const uint16 left_i2 = vertex_index++;
         const uint16 left_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  adjusted_step_height + first_step_offset,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_pos_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  adjusted_step_height + first_step_offset,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  0.0f,
                  half_steps_length,
               },
            .surface_index = stairway_surface_pos_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  half_steps_width,
                  0.0f,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_pos_x,
         });

         mesh.collision_triangles.push_back({left_i0, left_i1, left_i2});
         mesh.collision_triangles.push_back({left_i0, left_i2, left_i3});
         mesh.collision_occluders.push_back({left_i0, left_i1, left_i2, left_i3});

         const uint16 right_i0 = vertex_index++;
         const uint16 right_i1 = vertex_index++;
         const uint16 right_i2 = vertex_index++;
         const uint16 right_i3 = vertex_index++;

         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  0.0f,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  0.0f,
                  half_steps_length,
               },
            .surface_index = stairway_surface_neg_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  adjusted_step_height + first_step_offset,
                  half_steps_length,
               },
            .surface_index = stairway_surface_neg_x,
         });
         mesh.collision_vertices.push_back({
            .position =
               {
                  -half_steps_width,
                  adjusted_step_height + first_step_offset,
                  -half_steps_length,
               },
            .surface_index = stairway_surface_neg_x,
         });

         mesh.collision_triangles.push_back({right_i0, right_i1, right_i2});
         mesh.collision_triangles.push_back({right_i0, right_i2, right_i3});
         mesh.collision_occluders.push_back({right_i0, right_i1, right_i2, right_i3});
      }
   }

   // Snapping Mesh
   {
      const std::size_t point_count = 6;
      const std::size_t edge_count = 7;

      if (mesh.snap_points.capacity() != point_count) {
         mesh.snap_points = {};
         mesh.snap_points.reserve(point_count);
      }
      else {
         mesh.snap_points.clear();
      }

      if (mesh.snap_edges.capacity() != edge_count) {
         mesh.snap_edges = {};
         mesh.snap_edges.reserve(edge_count);
      }
      else {
         mesh.snap_edges.clear();
      }

      const float top = steps_height + first_step_offset;

      uint16 point_index = 0;

      const uint16 base_i0 = point_index++;
      const uint16 base_i1 = point_index++;
      const uint16 base_i2 = point_index++;
      const uint16 base_i3 = point_index++;

      mesh.snap_points.push_back({
         half_steps_width,
         0.0f,
         -half_steps_length,
      });
      mesh.snap_points.push_back({
         -half_steps_width,
         0.0f,
         -half_steps_length,
      });
      mesh.snap_points.push_back({
         -half_steps_width,
         0.0f,
         half_steps_length,
      });
      mesh.snap_points.push_back({
         half_steps_width,
         0.0f,
         half_steps_length,
      });

      const uint16 top_i0 = point_index++;
      const uint16 top_i1 = point_index++;

      mesh.snap_points.push_back({
         -half_steps_width,
         top,
         half_steps_length,
      });
      mesh.snap_points.push_back({
         half_steps_width,
         top,
         half_steps_length,
      });

      mesh.snap_edges.push_back({base_i0, base_i1});
      mesh.snap_edges.push_back({base_i1, base_i2});
      mesh.snap_edges.push_back({base_i2, base_i3});
      mesh.snap_edges.push_back({base_i3, base_i0});

      mesh.snap_edges.push_back({base_i2, top_i0});
      mesh.snap_edges.push_back({base_i3, top_i1});

      mesh.snap_edges.push_back({top_i0, top_i1});
   }

   return mesh;
}

auto generate_mesh(const block_custom_mesh_description_ring& ring) noexcept -> block_custom_mesh
{
   const float pi2 = std::numbers::pi_v<float> * 2.0f;
   const float inner_radius = ring.inner_radius;
   const float outer_radius = ring.outer_radius;
   const float height = ring.height;
   const int segments = ring.segments;
   const float ring_radius = inner_radius + outer_radius * 2.0f;
   const float texture_loops = ring.texture_loops;

   block_custom_mesh mesh;

   if (ring.flat_shading) {
      uint16 vertex_index = 0;

      mesh.vertices.reserve((2 + segments * 2) * 2 + (segments * 4 * 2));
      mesh.triangles.reserve(segments * 2 * 4);
      mesh.occluders.reserve(segments * 4);

      // Top Ring
      {
         uint16 last_outer_end_index = vertex_index++;
         uint16 last_inner_end_index = vertex_index++;

         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius + float3{0.0f, height, 0.0f},
            .normal = {0.0f, 1.0f, 0.0f},
            .texcoords = {0.0f, 0.0f},
            .surface_index = ring_surface_pos_y,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius +
                        float3{0.0f, height, 0.0f},
            .normal = float3{0.0f, 1.0f, 0.0f},
            .texcoords = {1.0f, 0.0f},
            .surface_index = ring_surface_pos_y,
         });

         for (int i = 1; i <= segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = last_outer_end_index;
            const uint16 i3 = last_inner_end_index;

            const float texcoord_y = circle_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS * inner_radius + float3{0.0f, height, 0.0f},
               .normal = float3{0.0f, 1.0f, 0.0f},
               .texcoords = {1.0f, texcoord_y},
               .surface_index = ring_surface_pos_y,
            });
            mesh.vertices.push_back({
               .position = endLS * ring_radius + float3{0.0f, height, 0.0f},
               .normal = float3{0.0f, 1.0f, 0.0f},
               .texcoords = {0.0f, texcoord_y},
               .surface_index = ring_surface_pos_y,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});

            last_outer_end_index = i1;
            last_inner_end_index = i0;
         }
      }

      // Bottom Ring
      {
         uint16 last_inner_end_index = vertex_index++;
         uint16 last_outer_end_index = vertex_index++;

         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius -
                        float3{0.0f, height, 0.0f},
            .normal = float3{0.0f, -1.0f, 0.0f},
            .texcoords = {1.0f, 0.0f},
            .surface_index = ring_surface_neg_y,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius - float3{0.0f, height, 0.0f},
            .normal = float3{0.0f, -1.0f, 0.0f},
            .texcoords = {0.0f, 0.0f},
            .surface_index = ring_surface_neg_y,
         });

         for (int i = 1; i <= segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = last_inner_end_index;
            const uint16 i1 = last_outer_end_index;
            const uint16 i2 = vertex_index++;
            const uint16 i3 = vertex_index++;

            const float texcoord_y = circle_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS * ring_radius - float3{0.0f, height, 0.0f},
               .normal = float3{0.0f, -1.0f, 0.0f},
               .texcoords = {0.0f, texcoord_y},
               .surface_index = ring_surface_neg_y,
            });
            mesh.vertices.push_back({
               .position = endLS * inner_radius - float3{0.0f, height, 0.0f},
               .normal = float3{0.0f, -1.0f, 0.0f},
               .texcoords = {1.0f, texcoord_y},
               .surface_index = ring_surface_neg_y,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});

            last_outer_end_index = i2;
            last_inner_end_index = i3;
         }
      }

      // Outer Ring
      {
         for (int i = 0; i < segments; ++i) {
            const float start_t = i / static_cast<float>(segments);
            const float mid_t = (i + 0.5f) / static_cast<float>(segments);
            const float end_t = (i + 1) / static_cast<float>(segments);
            const float segment_start = start_t * pi2;
            const float segment_mid = mid_t * pi2;
            const float segment_end = end_t * pi2;

            const float3 startLS = {
               cosf(segment_start),
               0.0f,
               sinf(segment_start),
            };
            const float3 midLS = {
               cosf(segment_mid),
               0.0f,
               sinf(segment_mid),
            };
            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = vertex_index++;
            const uint16 i3 = vertex_index++;

            const float texcoord_start_x = start_t * texture_loops;
            const float texcoord_end_x = end_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS * ring_radius + float3{0.0f, height, 0.0f},
               .normal = midLS,
               .texcoords = {texcoord_end_x, 1.0f},
               .surface_index = ring_surface_outer,
            });
            mesh.vertices.push_back({
               .position = endLS * ring_radius - float3{0.0f, height, 0.0f},
               .normal = midLS,
               .texcoords = {texcoord_end_x, 0.0f},
               .surface_index = ring_surface_outer,
            });
            mesh.vertices.push_back({
               .position = startLS * ring_radius - float3{0.0f, height, 0.0f},
               .normal = midLS,
               .texcoords = {texcoord_start_x, 0.0f},
               .surface_index = ring_surface_outer,
            });
            mesh.vertices.push_back({
               .position = startLS * ring_radius + float3{0.0f, height, 0.0f},
               .normal = midLS,
               .texcoords = {texcoord_start_x, 1.0f},
               .surface_index = ring_surface_outer,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});
         }
      }

      // Inner Ring
      {
         for (int i = 1; i <= segments; ++i) {
            const float start_t = i / static_cast<float>(segments);
            const float mid_t = (i + 0.5f) / static_cast<float>(segments);
            const float end_t = (i + 1) / static_cast<float>(segments);
            const float segment_start = start_t * pi2;
            const float segment_mid = mid_t * pi2;
            const float segment_end = end_t * pi2;

            const float3 startLS = {
               cosf(segment_start),
               0.0f,
               sinf(segment_start),
            };
            const float3 midLS = {
               cosf(segment_mid),
               0.0f,
               sinf(segment_mid),
            };
            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = vertex_index++;
            const uint16 i3 = vertex_index++;

            const float texcoord_start_x = start_t * texture_loops;
            const float texcoord_end_x = end_t * texture_loops;

            mesh.vertices.push_back({
               .position = startLS * inner_radius + float3{0.0f, height, 0.0f},
               .normal = -midLS,
               .texcoords = {texcoord_start_x, 1.0f},
               .surface_index = ring_surface_inner,
            });
            mesh.vertices.push_back({
               .position = startLS * inner_radius - float3{0.0f, height, 0.0f},
               .normal = -midLS,
               .texcoords = {texcoord_start_x, 0.0f},
               .surface_index = ring_surface_inner,
            });
            mesh.vertices.push_back({
               .position = endLS * inner_radius - float3{0.0f, height, 0.0f},
               .normal = -midLS,
               .texcoords = {texcoord_end_x, 0.0f},
               .surface_index = ring_surface_inner,
            });
            mesh.vertices.push_back({
               .position = endLS * inner_radius + float3{0.0f, height, 0.0f},
               .normal = -midLS,
               .texcoords = {texcoord_end_x, 1.0f},
               .surface_index = ring_surface_inner,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});
         }
      }
   }
   else {
      uint16 vertex_index = 0;

      mesh.vertices.reserve((2 + segments * 2) * 4);
      mesh.triangles.reserve(segments * 2 * 4);
      mesh.occluders.reserve(segments * 4);

      // Top Ring
      {
         uint16 last_outer_end_index = vertex_index++;
         uint16 last_inner_end_index = vertex_index++;

         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius + float3{0.0f, height, 0.0f},
            .normal = {0.0f, 1.0f, 0.0f},
            .texcoords = {0.0f, 0.0f},
            .surface_index = ring_surface_pos_y,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius +
                        float3{0.0f, height, 0.0f},
            .normal = float3{0.0f, 1.0f, 0.0f},
            .texcoords = {1.0f, 0.0f},
            .surface_index = ring_surface_pos_y,
         });

         for (int i = 1; i <= segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = last_outer_end_index;
            const uint16 i3 = last_inner_end_index;

            const float texcoord_y = circle_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS * inner_radius + float3{0.0f, height, 0.0f},
               .normal = float3{0.0f, 1.0f, 0.0f},
               .texcoords = {1.0f, texcoord_y},
               .surface_index = ring_surface_pos_y,
            });
            mesh.vertices.push_back({
               .position = endLS * ring_radius + float3{0.0f, height, 0.0f},
               .normal = float3{0.0f, 1.0f, 0.0f},
               .texcoords = {0.0f, texcoord_y},
               .surface_index = ring_surface_pos_y,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});

            last_outer_end_index = i1;
            last_inner_end_index = i0;
         }
      }

      // Bottom Ring
      {
         uint16 last_inner_end_index = vertex_index++;
         uint16 last_outer_end_index = vertex_index++;

         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius -
                        float3{0.0f, height, 0.0f},
            .normal = float3{0.0f, -1.0f, 0.0f},
            .texcoords = {1.0f, 0.0f},
            .surface_index = ring_surface_neg_y,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius - float3{0.0f, height, 0.0f},
            .normal = float3{0.0f, -1.0f, 0.0f},
            .texcoords = {0.0f, 0.0f},
            .surface_index = ring_surface_neg_y,
         });

         for (int i = 1; i <= segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = last_inner_end_index;
            const uint16 i1 = last_outer_end_index;
            const uint16 i2 = vertex_index++;
            const uint16 i3 = vertex_index++;

            const float texcoord_y = circle_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS * ring_radius - float3{0.0f, height, 0.0f},
               .normal = float3{0.0f, -1.0f, 0.0f},
               .texcoords = {0.0f, texcoord_y},
               .surface_index = ring_surface_neg_y,
            });
            mesh.vertices.push_back({
               .position = endLS * inner_radius - float3{0.0f, height, 0.0f},
               .normal = float3{0.0f, -1.0f, 0.0f},
               .texcoords = {1.0f, texcoord_y},
               .surface_index = ring_surface_neg_y,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});

            last_outer_end_index = i2;
            last_inner_end_index = i3;
         }
      }

      // Outer Ring
      {
         uint16 last_bottom_index = vertex_index++;
         uint16 last_top_index = vertex_index++;

         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius - float3{0.0f, height, 0.0f},
            .normal = float3{1.0f, 0.0f, 0.0f},
            .texcoords = {0.0f, 0.0f},
            .surface_index = ring_surface_outer,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius + float3{0.0f, height, 0.0f},
            .normal = float3{1.0f, 0.0f, 0.0f},
            .texcoords = {0.0f, 1.0f},
            .surface_index = ring_surface_outer,
         });

         for (int i = 1; i <= segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = last_bottom_index;
            const uint16 i3 = last_top_index;

            const float texcoord_x = circle_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS * ring_radius + float3{0.0f, height, 0.0f},
               .normal = endLS,
               .texcoords = {texcoord_x, 1.0f},
               .surface_index = ring_surface_outer,
            });
            mesh.vertices.push_back({
               .position = endLS * ring_radius - float3{0.0f, height, 0.0f},
               .normal = endLS,
               .texcoords = {texcoord_x, 0.0f},
               .surface_index = ring_surface_outer,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});

            last_top_index = i0;
            last_bottom_index = i1;
         }
      }

      // Inner Ring
      {
         uint16 last_top_index = vertex_index++;
         uint16 last_bottom_index = vertex_index++;

         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius +
                        float3{0.0f, height, 0.0f},
            .normal = float3{-1.0f, 0.0f, 0.0f},
            .texcoords = {0.0f, 1.0f},
            .surface_index = ring_surface_inner,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius -
                        float3{0.0f, height, 0.0f},
            .normal = float3{-1.0f, 0.0f, 0.0f},
            .texcoords = {0.0f, 0.0f},
            .surface_index = ring_surface_inner,
         });

         for (int i = 1; i <= segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = last_top_index;
            const uint16 i1 = last_bottom_index;
            const uint16 i2 = vertex_index++;
            const uint16 i3 = vertex_index++;

            const float texcoord_x = circle_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS * inner_radius - float3{0.0f, height, 0.0f},
               .normal = -endLS,
               .texcoords = {texcoord_x, 0.0f},
               .surface_index = ring_surface_inner,
            });
            mesh.vertices.push_back({
               .position = endLS * inner_radius + float3{0.0f, height, 0.0f},
               .normal = -endLS,
               .texcoords = {texcoord_x, 1.0f},
               .surface_index = ring_surface_inner,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});

            last_bottom_index = i2;
            last_top_index = i3;
         }
      }
   }

   // Collision Mesh
   {
      uint16 vertex_index = 0;

      mesh.collision_vertices.reserve(segments * 2 * 4);
      mesh.collision_triangles.reserve(segments * 2 * 4);
      mesh.collision_occluders.reserve(segments * 4);

      // Top Ring
      {
         const uint16 first_outer_end_index = vertex_index++;
         const uint16 first_inner_end_index = vertex_index++;

         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius + float3{0.0f, height, 0.0f},
            .surface_index = ring_surface_pos_y,
         });

         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius +
                        float3{0.0f, height, 0.0f},
            .surface_index = ring_surface_pos_y,
         });

         uint16 last_outer_end_index = first_outer_end_index;
         uint16 last_inner_end_index = first_inner_end_index;

         for (int i = 1; i < segments; ++i) {
            const float segment_end = (i / static_cast<float>(segments)) * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = last_outer_end_index;
            const uint16 i3 = last_inner_end_index;

            mesh.collision_vertices.push_back({
               .position = endLS * inner_radius + float3{0.0f, height, 0.0f},
               .surface_index = ring_surface_pos_y,
            });
            mesh.collision_vertices.push_back({
               .position = endLS * ring_radius + float3{0.0f, height, 0.0f},
               .surface_index = ring_surface_pos_y,
            });

            mesh.collision_triangles.push_back({i0, i1, i2});
            mesh.collision_triangles.push_back({i0, i2, i3});
            mesh.collision_occluders.push_back({i0, i1, i2, i3});

            last_outer_end_index = i1;
            last_inner_end_index = i0;
         }

         const uint16 i0 = first_inner_end_index;
         const uint16 i1 = first_outer_end_index;
         const uint16 i2 = last_outer_end_index;
         const uint16 i3 = last_inner_end_index;

         mesh.collision_triangles.push_back({i0, i1, i2});
         mesh.collision_triangles.push_back({i0, i2, i3});
         mesh.collision_occluders.push_back({i0, i1, i2, i3});
      }

      // Bottom Ring
      {
         const uint16 first_inner_end_index = vertex_index++;
         const uint16 first_outer_end_index = vertex_index++;

         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius -
                        float3{0.0f, height, 0.0f},
            .surface_index = ring_surface_neg_y,
         });
         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius - float3{0.0f, height, 0.0f},
            .surface_index = ring_surface_neg_y,
         });

         uint16 last_inner_end_index = first_inner_end_index;
         uint16 last_outer_end_index = first_outer_end_index;

         for (int i = 1; i < segments; ++i) {
            const float segment_end = (i / static_cast<float>(segments)) * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = last_inner_end_index;
            const uint16 i1 = last_outer_end_index;
            const uint16 i2 = vertex_index++;
            const uint16 i3 = vertex_index++;

            mesh.collision_vertices.push_back({
               .position = endLS * ring_radius - float3{0.0f, height, 0.0f},
               .surface_index = ring_surface_neg_y,
            });
            mesh.collision_vertices.push_back({
               .position = endLS * inner_radius - float3{0.0f, height, 0.0f},
               .surface_index = ring_surface_neg_y,
            });

            mesh.collision_triangles.push_back({i0, i1, i2});
            mesh.collision_triangles.push_back({i0, i2, i3});
            mesh.collision_occluders.push_back({i0, i1, i2, i3});

            last_outer_end_index = i2;
            last_inner_end_index = i3;
         }

         const uint16 i0 = last_inner_end_index;
         const uint16 i1 = last_outer_end_index;
         const uint16 i2 = first_outer_end_index;
         const uint16 i3 = first_inner_end_index;

         mesh.collision_triangles.push_back({i0, i1, i2});
         mesh.collision_triangles.push_back({i0, i2, i3});
         mesh.collision_occluders.push_back({i0, i1, i2, i3});
      }

      // Outer Ring
      {
         const uint16 first_bottom_index = vertex_index++;
         const uint16 first_top_index = vertex_index++;

         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius - float3{0.0f, height, 0.0f},
            .surface_index = ring_surface_outer,
         });
         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * ring_radius + float3{0.0f, height, 0.0f},
            .surface_index = ring_surface_outer,
         });

         uint16 last_bottom_index = first_bottom_index;
         uint16 last_top_index = first_top_index;

         for (int i = 1; i < segments; ++i) {
            const float segment_end = (i / static_cast<float>(segments)) * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = last_bottom_index;
            const uint16 i3 = last_top_index;

            mesh.collision_vertices.push_back({
               .position = endLS * ring_radius + float3{0.0f, height, 0.0f},
               .surface_index = ring_surface_outer,
            });
            mesh.collision_vertices.push_back({
               .position = endLS * ring_radius - float3{0.0f, height, 0.0f},
               .surface_index = ring_surface_outer,
            });

            mesh.collision_triangles.push_back({i0, i1, i2});
            mesh.collision_triangles.push_back({i0, i2, i3});
            mesh.collision_occluders.push_back({i0, i1, i2, i3});

            last_top_index = i0;
            last_bottom_index = i1;
         }

         const uint16 i0 = first_top_index;
         const uint16 i1 = first_bottom_index;
         const uint16 i2 = last_bottom_index;
         const uint16 i3 = last_top_index;

         mesh.collision_triangles.push_back({i0, i1, i2});
         mesh.collision_triangles.push_back({i0, i2, i3});
         mesh.collision_occluders.push_back({i0, i1, i2, i3});
      }

      // Inner Ring
      {
         const uint16 first_top_index = vertex_index++;
         const uint16 first_bottom_index = vertex_index++;

         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius +
                        float3{0.0f, height, 0.0f},
            .surface_index = ring_surface_inner,
         });
         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * inner_radius -
                        float3{0.0f, height, 0.0f},
            .surface_index = ring_surface_inner,
         });

         uint16 last_top_index = first_top_index;
         uint16 last_bottom_index = first_bottom_index;

         for (int i = 1; i < segments; ++i) {
            const float segment_end = (i / static_cast<float>(segments)) * pi2;

            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const uint16 i0 = last_top_index;
            const uint16 i1 = last_bottom_index;
            const uint16 i2 = vertex_index++;
            const uint16 i3 = vertex_index++;

            mesh.collision_vertices.push_back({
               .position = endLS * inner_radius - float3{0.0f, height, 0.0f},
               .surface_index = ring_surface_inner,
            });
            mesh.collision_vertices.push_back({
               .position = endLS * inner_radius + float3{0.0f, height, 0.0f},
               .surface_index = ring_surface_inner,
            });

            mesh.collision_triangles.push_back({i0, i1, i2});
            mesh.collision_triangles.push_back({i0, i2, i3});
            mesh.collision_occluders.push_back({i0, i1, i2, i3});

            last_bottom_index = i2;
            last_top_index = i3;
         }

         const uint16 i0 = last_top_index;
         const uint16 i1 = last_bottom_index;
         const uint16 i2 = first_bottom_index;
         const uint16 i3 = first_top_index;

         mesh.collision_triangles.push_back({i0, i1, i2});
         mesh.collision_triangles.push_back({i0, i2, i3});
         mesh.collision_occluders.push_back({i0, i1, i2, i3});
      }
   }

   // Snapping Mesh
   {
      mesh.snap_points.reserve(segments * 4);
      mesh.snap_edges.reserve(segments * 8);

      for (int i = 0; i < segments; ++i) {
         const float circle_t = i / static_cast<float>(segments);
         const float segment = circle_t * pi2;

         const float3 pointLS = {
            cosf(segment),
            0.0f,
            sinf(segment),
         };

         mesh.snap_points.push_back(pointLS * inner_radius + float3{0.0f, height, 0.0f});
         mesh.snap_points.push_back(pointLS * ring_radius + float3{0.0f, height, 0.0f});
         mesh.snap_points.push_back(pointLS * inner_radius - float3{0.0f, height, 0.0f});
         mesh.snap_points.push_back(pointLS * ring_radius - float3{0.0f, height, 0.0f});
      }

      for (uint16 i = 0; i < segments; ++i) {
         const uint16 pos_y_inner = i * 4 + 0;
         const uint16 pos_y_outer = i * 4 + 1;
         const uint16 neg_y_inner = i * 4 + 2;
         const uint16 neg_y_outer = i * 4 + 3;

         const uint16 next_pos_y_inner = ((i + 1) % segments) * 4 + 0;
         const uint16 next_pos_y_outer = ((i + 1) % segments) * 4 + 1;
         const uint16 next_neg_y_inner = ((i + 1) % segments) * 4 + 2;
         const uint16 next_neg_y_outer = ((i + 1) % segments) * 4 + 3;

         mesh.snap_edges.push_back({pos_y_inner, pos_y_outer});
         mesh.snap_edges.push_back({neg_y_inner, neg_y_outer});
         mesh.snap_edges.push_back({pos_y_inner, neg_y_inner});
         mesh.snap_edges.push_back({pos_y_outer, neg_y_outer});

         mesh.snap_edges.push_back({pos_y_inner, next_pos_y_inner});
         mesh.snap_edges.push_back({pos_y_outer, next_pos_y_outer});
         mesh.snap_edges.push_back({neg_y_inner, next_neg_y_inner});
         mesh.snap_edges.push_back({neg_y_outer, next_neg_y_outer});
      }
   }

   return mesh;
}

auto generate_mesh(const block_custom_mesh_description_beveled_box& box) noexcept
   -> block_custom_mesh
{
   const float3 size = box.size;
   const float amount =
      std::max(std::min(std::min(std::min(box.size.x, box.size.y), box.size.z),
                        box.amount),
               0.0f);

   block_custom_mesh mesh;

   const float top_amount = box.bevel_top ? amount : 0.0f;
   const float sides_amount = box.bevel_sides ? amount : 0.0f;
   const float bottom_amount = box.bevel_bottom ? amount : 0.0f;

   const std::array<float3, 4> top = {
      float3{size.x - amount, size.y, -size.z + amount},
      float3{-size.x + amount, size.y, -size.z + amount},
      float3{-size.x + amount, size.y, size.z - amount},
      float3{size.x - amount, size.y, size.z - amount},
   };

   const std::array<float3, 4> bottom = {
      float3{size.x - amount, -size.y, size.z - amount},
      float3{-size.x + amount, -size.y, size.z - amount},
      float3{-size.x + amount, -size.y, -size.z + amount},
      float3{size.x - amount, -size.y, -size.z + amount},
   };

   const std::array<float3, 4> left = {
      float3{-size.x, size.y - top_amount, -size.z + sides_amount},
      float3{-size.x, -size.y + bottom_amount, -size.z + sides_amount},
      float3{-size.x, -size.y + bottom_amount, size.z - sides_amount},
      float3{-size.x, size.y - top_amount, size.z - sides_amount},
   };

   const std::array<float3, 4> right = {
      float3{size.x, size.y - top_amount, size.z - sides_amount},
      float3{size.x, -size.y + bottom_amount, size.z - sides_amount},
      float3{size.x, -size.y + bottom_amount, -size.z + sides_amount},
      float3{size.x, size.y - top_amount, -size.z + sides_amount},
   };

   const std::array<float3, 4> front = {
      float3{size.x - sides_amount, size.y - top_amount, size.z},
      float3{-size.x + sides_amount, size.y - top_amount, size.z},
      float3{-size.x + sides_amount, -size.y + bottom_amount, size.z},
      float3{size.x - sides_amount, -size.y + bottom_amount, size.z},
   };

   const std::array<float3, 4> back = {

      float3{size.x - sides_amount, -size.y + bottom_amount, -size.z},
      float3{-size.x + sides_amount, -size.y + bottom_amount, -size.z},
      float3{-size.x + sides_amount, size.y - top_amount, -size.z},
      float3{size.x - sides_amount, size.y - top_amount, -size.z},
   };

   // Visible Mesh
   {
      // 96 verts, 44 tris, 18 occluders

      uint16 vertex_index = 0;

      mesh.vertices.reserve(box.bevel_sides ? 96 : 56);
      mesh.triangles.reserve(box.bevel_sides ? 44 : 28);
      mesh.occluders.reserve(box.bevel_sides ? 18 : 14);

      // Top
      {
         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = top[0],
            .normal = {0.0f, 1.0f, 0.0f},
            .texcoords = float2{top[0].z / size.z, top[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_y,
         });
         mesh.vertices.push_back({
            .position = top[1],
            .normal = {0.0f, 1.0f, 0.0f},
            .texcoords = float2{top[1].z / size.z, top[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_y,
         });
         mesh.vertices.push_back({
            .position = top[2],
            .normal = {0.0f, 1.0f, 0.0f},
            .texcoords = float2{top[2].z / size.z, top[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_y,
         });
         mesh.vertices.push_back({
            .position = top[3],
            .normal = {0.0f, 1.0f, 0.0f},
            .texcoords = float2{top[3].z / size.z, top[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_y,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Bottom
      {
         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = bottom[0],
            .normal = {0.0f, -1.0f, 0.0f},
            .texcoords = float2{bottom[0].z / size.z, bottom[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_y,
         });
         mesh.vertices.push_back({
            .position = bottom[1],
            .normal = {0.0f, -1.0f, 0.0f},
            .texcoords = float2{bottom[1].z / size.z, bottom[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_y,
         });
         mesh.vertices.push_back({
            .position = bottom[2],
            .normal = {0.0f, -1.0f, 0.0f},
            .texcoords = float2{bottom[2].z / size.z, bottom[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_y,
         });
         mesh.vertices.push_back({
            .position = bottom[3],
            .normal = {0.0f, -1.0f, 0.0f},
            .texcoords = float2{bottom[3].z / size.z, bottom[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_y,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Left
      {
         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = left[0],
            .normal = {-1.0f, 0.0f, 0.0f},
            .texcoords = float2{left[0].z / size.z, left[0].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_x,
         });
         mesh.vertices.push_back({
            .position = left[1],
            .normal = {-1.0f, 0.0f, 0.0f},
            .texcoords = float2{left[1].z / size.z, left[1].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_x,
         });
         mesh.vertices.push_back({
            .position = left[2],
            .normal = {-1.0f, 0.0f, 0.0f},
            .texcoords = float2{left[2].z / size.z, left[2].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_x,
         });
         mesh.vertices.push_back({
            .position = left[3],
            .normal = {-1.0f, 0.0f, 0.0f},
            .texcoords = float2{left[3].z / size.z, left[3].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_x,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Right
      {
         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = right[0],
            .normal = {1.0f, 0.0f, 0.0f},
            .texcoords = float2{right[0].z / size.z, right[0].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_x,
         });
         mesh.vertices.push_back({
            .position = right[1],
            .normal = {1.0f, 0.0f, 0.0f},
            .texcoords = float2{right[1].z / size.z, right[1].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_x,
         });
         mesh.vertices.push_back({
            .position = right[2],
            .normal = {1.0f, 0.0f, 0.0f},
            .texcoords = float2{right[2].z / size.z, right[2].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_x,
         });
         mesh.vertices.push_back({
            .position = right[3],
            .normal = {1.0f, 0.0f, 0.0f},
            .texcoords = float2{right[3].z / size.z, right[3].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_x,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Front
      {
         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = front[0],
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoords = float2{front[0].x / size.x, front[0].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_z,
         });
         mesh.vertices.push_back({
            .position = front[1],
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoords = float2{front[1].x / size.x, front[1].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_z,
         });
         mesh.vertices.push_back({
            .position = front[2],
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoords = float2{front[2].x / size.x, front[2].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_z,
         });
         mesh.vertices.push_back({
            .position = front[3],
            .normal = {0.0f, 0.0f, 1.0f},
            .texcoords = float2{front[3].x / size.x, front[3].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_z,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Back
      {
         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = back[0],
            .normal = {0.0f, 0.0f, -1.0f},
            .texcoords = float2{back[0].x / size.x, back[0].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_z,
         });
         mesh.vertices.push_back({
            .position = back[1],
            .normal = {0.0f, 0.0f, -1.0f},
            .texcoords = float2{back[1].x / size.x, back[1].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_z,
         });
         mesh.vertices.push_back({
            .position = back[2],
            .normal = {0.0f, 0.0f, -1.0f},
            .texcoords = float2{back[2].x / size.x, back[2].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_z,
         });
         mesh.vertices.push_back({
            .position = back[3],
            .normal = {0.0f, 0.0f, -1.0f},
            .texcoords = float2{back[3].x / size.x, back[3].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_z,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Top Left
      {
         const float3 normal = calculate_normal(top[2], top[1], left[0]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };
         mesh.vertices.push_back({
            .position = top[2],
            .normal = normal,
            .texcoords = float2{top[2].z / size.z, top[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = top[1],
            .normal = normal,
            .texcoords = float2{top[1].z / size.z, top[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = left[0],
            .normal = normal,
            .texcoords = float2{left[0].z / size.z, left[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = left[3],
            .normal = normal,
            .texcoords = float2{left[3].z / size.z, left[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Top Right
      {
         const float3 normal = calculate_normal(right[0], right[3], top[0]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = right[0],
            .normal = normal,
            .texcoords = float2{right[0].z / size.z, right[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = right[3],
            .normal = normal,
            .texcoords = float2{right[3].z / size.z, right[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = top[0],
            .normal = normal,
            .texcoords = float2{top[0].z / size.z, top[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = top[3],
            .normal = normal,
            .texcoords = float2{top[3].z / size.z, top[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Top Front
      {
         const float3 normal = calculate_normal(front[1], front[0], top[3]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = front[1],
            .normal = normal,
            .texcoords = float2{front[1].z / size.z, front[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = front[0],
            .normal = normal,
            .texcoords = float2{front[0].z / size.z, front[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = top[3],
            .normal = normal,
            .texcoords = float2{top[3].z / size.z, top[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = top[2],
            .normal = normal,
            .texcoords = float2{top[2].z / size.z, top[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Top Back
      {
         const float3 normal = calculate_normal(top[1], top[0], back[3]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = top[1],
            .normal = normal,
            .texcoords = float2{top[1].z / size.z, top[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = top[0],
            .normal = normal,
            .texcoords = float2{top[0].z / size.z, top[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = back[3],
            .normal = normal,
            .texcoords = float2{back[3].z / size.z, back[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = back[2],
            .normal = normal,
            .texcoords = float2{back[2].z / size.z, back[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Top Left Front
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(top[2], left[3], front[1]);

         const std::array<uint16, 3> tri = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = top[2],
            .normal = normal,
            .texcoords = float2{top[2].z / size.z, top[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = left[3],
            .normal = normal,
            .texcoords = float2{left[3].z / size.z, left[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = front[1],
            .normal = normal,
            .texcoords = float2{front[1].z / size.z, front[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });

         mesh.triangles.push_back(tri);
      }

      // Top Right Front
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(front[0], right[0], top[3]);

         const std::array<uint16, 3> tri = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = front[0],
            .normal = normal,
            .texcoords = float2{front[0].z / size.z, front[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = right[0],
            .normal = normal,
            .texcoords = float2{right[0].z / size.z, right[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = top[3],
            .normal = normal,
            .texcoords = float2{top[3].z / size.z, top[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });

         mesh.triangles.push_back(tri);
      }

      // Top Left Back
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(back[2], left[0], top[1]);

         const std::array<uint16, 3> tri = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = back[2],
            .normal = normal,
            .texcoords = float2{back[2].z / size.z, back[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = left[0],
            .normal = normal,
            .texcoords = float2{left[0].z / size.z, left[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = top[1],
            .normal = normal,
            .texcoords = float2{top[1].z / size.z, top[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });

         mesh.triangles.push_back(tri);
      }

      // Top Right Back
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(top[0], right[3], back[3]);

         const std::array<uint16, 3> tri = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = top[0],
            .normal = normal,
            .texcoords = float2{top[0].z / size.z, top[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = right[3],
            .normal = normal,
            .texcoords = float2{right[3].z / size.z, right[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });
         mesh.vertices.push_back({
            .position = back[3],
            .normal = normal,
            .texcoords = float2{back[3].z / size.z, back[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_top_edge,
         });

         mesh.triangles.push_back(tri);
      }

      // Bottom Left
      {
         const float3 normal = calculate_normal(left[2], left[1], bottom[2]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = left[2],
            .normal = normal,
            .texcoords = float2{left[2].z / size.z, left[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = left[1],
            .normal = normal,
            .texcoords = float2{left[1].z / size.z, left[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = bottom[2],
            .normal = normal,
            .texcoords = float2{bottom[2].z / size.z, bottom[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = bottom[1],
            .normal = normal,
            .texcoords = float2{bottom[1].z / size.z, bottom[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Bottom Right
      {
         const float3 normal = calculate_normal(bottom[0], bottom[3], right[2]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = bottom[0],
            .normal = normal,
            .texcoords = float2{bottom[0].z / size.z, bottom[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = bottom[3],
            .normal = normal,
            .texcoords = float2{bottom[3].z / size.z, bottom[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = right[2],
            .normal = normal,
            .texcoords = float2{right[2].z / size.z, right[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = right[1],
            .normal = normal,
            .texcoords = float2{right[1].z / size.z, right[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Bottom Front
      {
         const float3 normal = calculate_normal(bottom[1], bottom[0], front[3]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = bottom[1],
            .normal = normal,
            .texcoords = float2{bottom[1].z / size.z, bottom[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = bottom[0],
            .normal = normal,
            .texcoords = float2{bottom[0].z / size.z, bottom[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = front[3],
            .normal = normal,
            .texcoords = float2{front[3].z / size.z, front[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = front[2],
            .normal = normal,
            .texcoords = float2{front[2].z / size.z, front[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Bottom Back
      {
         const float3 normal = calculate_normal(back[1], back[0], bottom[3]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = back[1],
            .normal = normal,
            .texcoords = float2{back[1].z / size.z, back[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = back[0],
            .normal = normal,
            .texcoords = float2{back[0].z / size.z, back[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = bottom[3],
            .normal = normal,
            .texcoords = float2{bottom[3].z / size.z, bottom[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = bottom[2],
            .normal = normal,
            .texcoords = float2{bottom[2].z / size.z, bottom[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      // Bottom Left Front
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(front[2], left[2], bottom[1]);

         const std::array<uint16, 3> tri = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = front[2],
            .normal = normal,
            .texcoords = float2{front[2].z / size.z, front[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = left[2],
            .normal = normal,
            .texcoords = float2{left[2].z / size.z, left[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = bottom[1],
            .normal = normal,
            .texcoords = float2{bottom[1].z / size.z, bottom[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });

         mesh.triangles.push_back(tri);
      }

      // Bottom Right Front
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(bottom[0], right[1], front[3]);

         const std::array<uint16, 3> tri = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = bottom[0],
            .normal = normal,
            .texcoords = float2{bottom[0].z / size.z, bottom[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = right[1],
            .normal = normal,
            .texcoords = float2{right[1].z / size.z, right[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = front[3],
            .normal = normal,
            .texcoords = float2{front[3].z / size.z, front[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });

         mesh.triangles.push_back(tri);
      }

      // Bottom Left Back
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(bottom[2], left[1], back[1]);

         const std::array<uint16, 3> tri = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = bottom[2],
            .normal = normal,
            .texcoords = float2{front[2].z / size.z, front[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = left[1],
            .normal = normal,
            .texcoords = float2{left[1].z / size.z, left[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = back[1],
            .normal = normal,
            .texcoords = float2{back[1].z / size.z, back[1].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });

         mesh.triangles.push_back(tri);
      }

      // Bottom Right Back
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(back[0], right[2], bottom[3]);

         const std::array<uint16, 3> tri = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = back[0],
            .normal = normal,
            .texcoords = float2{back[0].z / size.z, back[0].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = right[2],
            .normal = normal,
            .texcoords = float2{right[2].z / size.z, right[2].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });
         mesh.vertices.push_back({
            .position = bottom[3],
            .normal = normal,
            .texcoords = float2{bottom[3].z / size.z, bottom[3].x / size.x} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_bottom_edge,
         });

         mesh.triangles.push_back(tri);
      }

      // Left Front
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(front[2], front[1], left[3]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = front[2],
            .normal = normal,
            .texcoords = float2{front[2].z / size.z, front[2].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = front[1],
            .normal = normal,
            .texcoords = float2{front[1].z / size.z, front[1].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = left[3],
            .normal = normal,
            .texcoords = float2{left[3].z / size.z, left[3].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = left[2],
            .normal = normal,
            .texcoords = float2{left[2].z / size.z, left[2].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      //  Right Front
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(front[0], front[3], right[1]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = front[0],
            .normal = normal,
            .texcoords = float2{front[0].z / size.z, front[0].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = front[3],
            .normal = normal,
            .texcoords = float2{front[3].z / size.z, front[3].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = right[1],
            .normal = normal,
            .texcoords = float2{right[1].z / size.z, right[1].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = right[0],
            .normal = normal,
            .texcoords = float2{right[0].z / size.z, right[0].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      //  Left Back
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(back[2], back[1], left[1]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = back[2],
            .normal = normal,
            .texcoords = float2{back[2].z / size.z, back[2].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = back[1],
            .normal = normal,
            .texcoords = float2{back[1].z / size.z, back[1].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = left[1],
            .normal = normal,
            .texcoords = float2{left[1].z / size.z, left[1].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = left[0],
            .normal = normal,
            .texcoords = float2{left[0].z / size.z, left[0].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }

      //  Right Back
      if (box.bevel_sides) {
         const float3 normal = calculate_normal(right[2], back[0], back[3]);

         const std::array<uint16, 4> quad = {
            vertex_index++,
            vertex_index++,
            vertex_index++,
            vertex_index++,
         };

         mesh.vertices.push_back({
            .position = right[2],
            .normal = normal,
            .texcoords = float2{right[2].z / size.z, right[2].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = back[0],
            .normal = normal,
            .texcoords = float2{back[0].z / size.z, back[0].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = back[3],
            .normal = normal,
            .texcoords = float2{back[3].z / size.z, back[3].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });
         mesh.vertices.push_back({
            .position = right[3],
            .normal = normal,
            .texcoords = float2{right[3].z / size.z, right[3].y / size.y} * 0.5f + 0.5f,
            .surface_index = beveled_box_surface_side_edge,
         });

         mesh.triangles.push_back({quad[0], quad[1], quad[2]});
         mesh.triangles.push_back({quad[0], quad[2], quad[3]});

         mesh.occluders.push_back(quad);
      }
   }

   mesh.collision_vertices.reserve(mesh.vertices.size());

   for (const block_vertex& vertex : mesh.vertices) {
      mesh.collision_vertices.push_back(
         {.position = vertex.position, .surface_index = vertex.surface_index});
   }

   mesh.collision_triangles = mesh.triangles;
   mesh.collision_occluders = mesh.occluders;

   // Snapping Points
   {
      mesh.snap_points = {// top corners
                          float3{box.size.x, box.size.y, box.size.z},
                          float3{-box.size.x, box.size.y, box.size.z},
                          float3{-box.size.x, box.size.y, -box.size.z},
                          float3{box.size.x, box.size.y, -box.size.z},
                          // bottom corners
                          float3{box.size.x, -box.size.y, box.size.z},
                          float3{-box.size.x, -box.size.y, box.size.z},
                          float3{-box.size.x, -box.size.y, -box.size.z},
                          float3{box.size.x, -box.size.y, -box.size.z}};

      mesh.snap_edges = {
         {0, 1}, {1, 2}, {2, 3}, {3, 0}, //
         {4, 5}, {5, 6}, {6, 7}, {7, 5}, //
         {0, 4}, {1, 5}, {2, 6}, {3, 7}, //
      };
   }

   return mesh;
}

auto generate_mesh(const block_custom_mesh_description_curve& curve) noexcept -> block_custom_mesh
{
   const float3 p0 = curve.p0;
   const float3 p1 = curve.p1;
   const float3 p2 = curve.p2;
   const float3 p3 = curve.p3;
   const float width = curve.width;
   const float half_width = width / 2.0f;
   const float height = curve.height;
   const float texture_loops = curve.texture_loops;
   const int segments = curve.segments;
   const float segments_flt = static_cast<float>(segments);

   float curve_length = 0.0f;

   for (int i = 0; i < segments; ++i) {
      const float3 start = cubic_bezier(p0, p1, p2, p3, i / segments_flt);
      const float3 end = cubic_bezier(p0, p1, p2, p3, (i + 1) / segments_flt);

      curve_length += distance(start, end);
   }

   block_custom_mesh mesh;

   mesh.vertices.reserve((segments + 1) * 8 + 8);
   mesh.occluders.reserve(segments * 4 + 2);

   float3 last_position = p0;
   float traveled_length = 0.0f;

   for (int i = 0; i <= segments; ++i) {
      const float3 position = cubic_bezier(p0, p1, p2, p3, i / segments_flt);
      const float3 tangent =
         cubic_bezier_tangent(p0, p1, p2, p3,
                              std::max(std::min(i / segments_flt, 1.0f - FLT_EPSILON),
                                       FLT_EPSILON));
      const float3 normal = calculate_curve_axis_normal(tangent, {0.0f, 1.0f, 0.0f});

      const float3 x_axis = normalize(cross(tangent, normal));
      const float3 y_axis = normalize(cross(x_axis, tangent));
      const float3 z_axis = tangent;

      const float3 top = position + y_axis * height;
      const float3 bottom = position;
      const float3 left = position - x_axis * half_width;
      const float3 right = position + x_axis * half_width;

      traveled_length += distance(position, last_position);
      last_position = position;

      const float texcoord = (traveled_length / curve_length) * texture_loops;

      mesh.vertices.push_back({
         .position = left + y_axis * height,
         .normal = y_axis,
         .texcoords = {0.0f, texcoord},
         .surface_index = curve_surface_pos_y,
      });

      mesh.vertices.push_back({
         .position = right + y_axis * height,
         .normal = y_axis,
         .texcoords = {1.0f, texcoord},
         .surface_index = curve_surface_pos_y,
      });

      mesh.vertices.push_back({
         .position = left,
         .normal = -y_axis,
         .texcoords = {0.0f, texcoord},
         .surface_index = curve_surface_neg_y,
      });

      mesh.vertices.push_back({
         .position = right,
         .normal = -y_axis,
         .texcoords = {1.0f, texcoord},
         .surface_index = curve_surface_neg_y,
      });

      mesh.vertices.push_back({
         .position = top - x_axis * half_width,
         .normal = -x_axis,
         .texcoords = {texcoord, 1.0f},
         .surface_index = curve_surface_neg_x,
      });

      mesh.vertices.push_back({
         .position = bottom - x_axis * half_width,
         .normal = -x_axis,
         .texcoords = {texcoord, 0.0f},
         .surface_index = curve_surface_neg_x,
      });

      mesh.vertices.push_back({
         .position = top + x_axis * half_width,
         .normal = x_axis,
         .texcoords = {texcoord, 1.0f},
         .surface_index = curve_surface_pos_x,
      });

      mesh.vertices.push_back({
         .position = bottom + x_axis * half_width,
         .normal = x_axis,
         .texcoords = {texcoord, 0.0f},
         .surface_index = curve_surface_pos_x,
      });
   }

   for (int i = 0; i < segments; ++i) {
      const int segment_start = i * 8;
      const int segment_end = (i + 1) * 8;

      if (width > 0.0f) {
         // Y+
         mesh.occluders.push_back({
            static_cast<uint16>(segment_start + 0),
            static_cast<uint16>(segment_start + 1),
            static_cast<uint16>(segment_end + 1),
            static_cast<uint16>(segment_end + 0),
         });

         // Y-
         mesh.occluders.push_back({
            static_cast<uint16>(segment_end + 2),
            static_cast<uint16>(segment_end + 3),
            static_cast<uint16>(segment_start + 3),
            static_cast<uint16>(segment_start + 2),
         });
      }

      // X-
      mesh.occluders.push_back({
         static_cast<uint16>(segment_end + 4),
         static_cast<uint16>(segment_end + 5),
         static_cast<uint16>(segment_start + 5),
         static_cast<uint16>(segment_start + 4),
      });

      // X+
      mesh.occluders.push_back({
         static_cast<uint16>(segment_start + 6),
         static_cast<uint16>(segment_start + 7),
         static_cast<uint16>(segment_end + 7),
         static_cast<uint16>(segment_end + 6),
      });
   }

   if (width > 0.0f and height > 0.0f) {
      // Back
      {
         const float3 position = cubic_bezier(p0, p1, p2, p3, 0.0f);
         const float3 tangent = cubic_bezier_tangent(p0, p1, p2, p3, FLT_EPSILON);
         const float3 normal =
            calculate_curve_axis_normal(tangent, {0.0f, 1.0f, 0.0f});

         const float3 x_axis = normalize(cross(tangent, normal));
         const float3 y_axis = normalize(cross(x_axis, tangent));
         const float3 z_axis = tangent;

         const float3 top = position + y_axis * height;
         const float3 bottom = position;

         const float3 top_left = top - x_axis * half_width;
         const float3 top_right = top + x_axis * half_width;

         const float3 bottom_left = bottom - x_axis * half_width;
         const float3 bottom_right = bottom + x_axis * half_width;

         mesh.occluders.push_back({
            static_cast<uint16>(mesh.vertices.size() + 0),
            static_cast<uint16>(mesh.vertices.size() + 1),
            static_cast<uint16>(mesh.vertices.size() + 2),
            static_cast<uint16>(mesh.vertices.size() + 3),
         });

         mesh.vertices.push_back({
            .position = bottom_left,
            .normal = -z_axis,
            .texcoords = {0.0f, 1.0f},
            .surface_index = curve_surface_neg_z,
         });
         mesh.vertices.push_back({
            .position = bottom_right,
            .normal = -z_axis,
            .texcoords = {1.0f, 1.0f},
            .surface_index = curve_surface_neg_z,
         });
         mesh.vertices.push_back({
            .position = top_right,
            .normal = -z_axis,
            .texcoords = {1.0f, 0.0f},
            .surface_index = curve_surface_neg_z,
         });
         mesh.vertices.push_back({
            .position = top_left,
            .normal = -z_axis,
            .texcoords = {0.0f, 0.0f},
            .surface_index = curve_surface_neg_z,
         });
      }

      // Front
      {
         const float3 position = cubic_bezier(p0, p1, p2, p3, 1.0f);
         const float3 tangent =
            cubic_bezier_tangent(p0, p1, p2, p3, 1.0f - FLT_EPSILON);
         const float3 normal =
            calculate_curve_axis_normal(tangent, {0.0f, 1.0f, 0.0f});

         const float3 x_axis = normalize(cross(tangent, normal));
         const float3 y_axis = normalize(cross(x_axis, tangent));
         const float3 z_axis = tangent;

         const float3 top = position + y_axis * height;
         const float3 bottom = position;

         const float3 top_left = top - x_axis * half_width;
         const float3 top_right = top + x_axis * half_width;

         const float3 bottom_left = bottom - x_axis * half_width;
         const float3 bottom_right = bottom + x_axis * half_width;

         mesh.occluders.push_back({
            static_cast<uint16>(mesh.vertices.size() + 0),
            static_cast<uint16>(mesh.vertices.size() + 1),
            static_cast<uint16>(mesh.vertices.size() + 2),
            static_cast<uint16>(mesh.vertices.size() + 3),
         });

         mesh.vertices.push_back({
            .position = top_left,
            .normal = z_axis,
            .texcoords = {0.0f, 0.0f},
            .surface_index = curve_surface_neg_z,
         });
         mesh.vertices.push_back({
            .position = top_right,
            .normal = z_axis,
            .texcoords = {1.0f, 0.0f},
            .surface_index = curve_surface_neg_z,
         });
         mesh.vertices.push_back({
            .position = bottom_right,
            .normal = z_axis,
            .texcoords = {1.0f, 1.0f},
            .surface_index = curve_surface_neg_z,
         });
         mesh.vertices.push_back({
            .position = bottom_left,
            .normal = z_axis,
            .texcoords = {0.0f, 1.0f},
            .surface_index = curve_surface_neg_z,
         });
      }
   }

   mesh.triangles.reserve(mesh.occluders.size() * 2);

   for (const auto [i0, i1, i2, i3] : mesh.occluders) {
      mesh.triangles.push_back({i0, i1, i2});
      mesh.triangles.push_back({i0, i2, i3});
   }

   mesh.collision_vertices.reserve(mesh.vertices.size());

   for (const block_vertex& vertex : mesh.vertices) {
      mesh.collision_vertices.push_back(
         {.position = vertex.position, .surface_index = vertex.surface_index});
   }

   mesh.collision_triangles = mesh.triangles;
   mesh.collision_occluders = mesh.occluders;

   mesh.snap_points.reserve((segments + 1) * 4);
   mesh.snap_edges.reserve(segments * 4 + 8);

   // 0, 1, 2, 3
   for (int i = 0; i <= segments; ++i) {
      mesh.snap_points.push_back(mesh.vertices[i * 8 + 0].position);
      mesh.snap_points.push_back(mesh.vertices[i * 8 + 1].position);
      mesh.snap_points.push_back(mesh.vertices[i * 8 + 2].position);
      mesh.snap_points.push_back(mesh.vertices[i * 8 + 3].position);
   }

   for (int i = 0; i < segments; ++i) {
      const int segment_start = i * 4;
      const int segment_end = (i + 1) * 4;

      mesh.snap_edges.push_back({
         static_cast<uint16>(segment_start + 0),
         static_cast<uint16>(segment_end + 0),
      });

      if (width > 0.0f) {
         mesh.snap_edges.push_back({
            static_cast<uint16>(segment_start + 1),
            static_cast<uint16>(segment_end + 1),
         });

         mesh.snap_edges.push_back({
            static_cast<uint16>(segment_start + 2),
            static_cast<uint16>(segment_end + 2),
         });
      }

      mesh.snap_edges.push_back({
         static_cast<uint16>(segment_start + 3),
         static_cast<uint16>(segment_end + 3),
      });
   }

   mesh.snap_edges.push_back({0, 1});
   mesh.snap_edges.push_back({1, 3});
   mesh.snap_edges.push_back({2, 3});
   mesh.snap_edges.push_back({2, 0});

   mesh.snap_edges.push_back({static_cast<uint16>(segments * 4 + 0),
                              static_cast<uint16>(segments * 4 + 1)});
   mesh.snap_edges.push_back({static_cast<uint16>(segments * 4 + 1),
                              static_cast<uint16>(segments * 4 + 3)});
   mesh.snap_edges.push_back({static_cast<uint16>(segments * 4 + 2),
                              static_cast<uint16>(segments * 4 + 3)});
   mesh.snap_edges.push_back({static_cast<uint16>(segments * 4 + 2),
                              static_cast<uint16>(segments * 4 + 0)});

   return mesh;
}

auto generate_mesh(const block_custom_mesh_description_cylinder& cylinder) noexcept
   -> block_custom_mesh
{

   world::block_custom_mesh mesh;

   const float pi2 = std::numbers::pi_v<float> * 2.0f;
   const int segments = cylinder.segments;
   const float texture_loops = cylinder.texture_loops;
   const float cap_aspect = 1.0f / (cylinder.size.x / cylinder.size.z);
   const float3 size = cylinder.size;

   const float3x3 local_from_circle_adjugate = adjugate({
      {size.x, 0.0f, 0.0f, 0.0f},
      {0.0f, size.y, 0.0f, 0.0f},
      {0.0f, 0.0f, size.z, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
   });

   // Visible Mesh
   {
      uint16 vertex_index = 0;

      mesh.triangles.reserve(segments * 4);
      mesh.occluders.reserve(segments);

      // Wall
      if (cylinder.flat_shading) {
         mesh.vertices.reserve((segments * 4) + 2 + (segments * 2));

         for (int i = 0; i < segments; ++i) {
            const float start_circle_t = i / static_cast<float>(segments);
            const float segment_start = start_circle_t * pi2;
            const float mid_circle_t = (i + 0.5f) / static_cast<float>(segments);
            const float segment_mid = mid_circle_t * pi2;
            const float end_circle_t = (i + 1) / static_cast<float>(segments);
            const float segment_end = end_circle_t * pi2;

            const float3 startLS = {
               cosf(segment_start) * size.x,
               0.0f,
               sinf(segment_start) * size.z,
            };
            const float3 endLS = {
               cosf(segment_end) * size.x,
               0.0f,
               sinf(segment_end) * size.z,
            };

            const float3 normalCS = {
               cosf(segment_mid),
               0.0f,
               sinf(segment_mid),
            };
            const float3 normalLS = local_from_circle_adjugate * normalCS;

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = vertex_index++;
            const uint16 i3 = vertex_index++;

            const float start_texcoord_x = start_circle_t * texture_loops;
            const float end_texcoord_x = end_circle_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS + float3{0.0f, size.y, 0.0f},
               .normal = normalLS,
               .texcoords = {end_texcoord_x, 1.0f},
               .surface_index = cylinder_surface_wall,
            });
            mesh.vertices.push_back({
               .position = endLS - float3{0.0f, size.y, 0.0f},
               .normal = normalLS,
               .texcoords = {end_texcoord_x, 0.0f},
               .surface_index = cylinder_surface_wall,
            });
            mesh.vertices.push_back({
               .position = startLS - float3{0.0f, size.y, 0.0f},
               .normal = normalLS,
               .texcoords = {start_texcoord_x, 0.0f},
               .surface_index = cylinder_surface_wall,
            });
            mesh.vertices.push_back({
               .position = startLS + float3{0.0f, size.y, 0.0f},
               .normal = normalLS,
               .texcoords = {start_texcoord_x, 1.0f},
               .surface_index = cylinder_surface_wall,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});
         }
      }
      else {
         mesh.vertices.reserve(2 + (segments * 2) + 2 + (segments * 2));

         uint16 last_bottom_index = vertex_index++;
         uint16 last_top_index = vertex_index++;

         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * size.x - float3{0.0f, size.y, 0.0f},
            .normal = local_from_circle_adjugate * float3{1.0f, 0.0f, 0.0f},
            .texcoords = {0.0f, 0.0f},
            .surface_index = cylinder_surface_wall,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * size.x + float3{0.0f, size.y, 0.0f},
            .normal = local_from_circle_adjugate * float3{1.0f, 0.0f, 0.0f},
            .texcoords = {0.0f, 1.0f},
            .surface_index = cylinder_surface_wall,
         });

         for (int i = 1; i <= segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endCS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };
            const float3 endLS = endCS * float3{size.x, 0.0f, size.z};

            const float3 normalLS = local_from_circle_adjugate * endCS;

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = last_bottom_index;
            const uint16 i3 = last_top_index;

            const float texcoord_x = circle_t * texture_loops;

            mesh.vertices.push_back({
               .position = endLS + float3{0.0f, size.y, 0.0f},
               .normal = normalLS,
               .texcoords = {texcoord_x, 1.0f},
               .surface_index = cylinder_surface_wall,
            });
            mesh.vertices.push_back({
               .position = endLS - float3{0.0f, size.y, 0.0f},
               .normal = normalLS,
               .texcoords = {texcoord_x, 0.0f},
               .surface_index = cylinder_surface_wall,
            });

            mesh.triangles.push_back({i0, i1, i2});
            mesh.triangles.push_back({i0, i2, i3});
            mesh.occluders.push_back({i0, i1, i2, i3});

            last_top_index = i0;
            last_bottom_index = i1;
         }
      }

      // Top
      {
         const uint16 centre_index = vertex_index++;
         const uint16 start_index = vertex_index++;
         uint16 last_index = start_index;

         mesh.vertices.push_back({
            .position = float3{0.0f, size.y, 0.0f},
            .normal = float3{0.0f, 1.0f, 0.0f},
            .texcoords = {0.5f, 0.5f},
            .surface_index = cylinder_surface_pos_y,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * size.x + float3{0.0f, size.y, 0.0f},
            .normal = float3{0.0f, 1.0f, 0.0f},
            .texcoords = {1.0f, 0.5f},
            .surface_index = cylinder_surface_pos_y,
         });

         for (int i = 1; i < segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float tri_end = circle_t * pi2;

            const float3 pointCS = {
               cosf(tri_end),
               0.0f,
               sinf(tri_end),
            };
            const float3 pointLS = {
               pointCS.x * size.x,
               size.y,
               pointCS.z * size.z,
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = last_index;
            const uint16 i2 = centre_index;

            const float2 texcoords = {
               pointCS.x * 0.5f + 0.5f,
               (pointCS.z * cap_aspect * 0.5f + 0.5f),
            };

            mesh.vertices.push_back({
               .position = pointLS,
               .normal = float3{0.0f, 1.0f, 0.0f},
               .texcoords = texcoords,
               .surface_index = cylinder_surface_pos_y,
            });

            mesh.triangles.push_back({i0, i1, i2});

            last_index = i0;
         }

         mesh.triangles.push_back({start_index, last_index, centre_index});
      }

      // Bottom
      {
         const uint16 centre_index = vertex_index++;
         const uint16 start_index = vertex_index++;
         uint16 last_index = start_index;

         mesh.vertices.push_back({
            .position = float3{0.0f, -size.y, 0.0f},
            .normal = float3{0.0f, -1.0f, 0.0f},
            .texcoords = {0.5f, 0.5f},
            .surface_index = cylinder_surface_neg_y,
         });
         mesh.vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * size.x - float3{0.0f, size.y, 0.0f},
            .normal = float3{0.0f, -1.0f, 0.0f},
            .texcoords = {1.0f, 0.5f},
            .surface_index = cylinder_surface_neg_y,
         });

         for (int i = 1; i < segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float tri_end = circle_t * pi2;

            const float3 pointCS = {
               cosf(tri_end),
               0.0f,
               sinf(tri_end),
            };
            const float3 pointLS = {
               pointCS.x * size.x,
               -size.y,
               pointCS.z * size.z,
            };

            const uint16 i0 = centre_index;
            const uint16 i1 = last_index;
            const uint16 i2 = vertex_index++;

            const float2 texcoords = {
               pointCS.x * 0.5f + 0.5f,
               (pointCS.z * cap_aspect * 0.5f + 0.5f),
            };

            mesh.vertices.push_back({
               .position = pointLS,
               .normal = float3{0.0f, -1.0f, 0.0f},
               .texcoords = texcoords,
               .surface_index = cylinder_surface_neg_y,
            });

            mesh.triangles.push_back({i0, i1, i2});

            last_index = i2;
         }

         mesh.triangles.push_back({centre_index, last_index, start_index});
      }
   }

   // Collision Mesh
   {
      uint16 vertex_index = 0;

      mesh.collision_triangles.reserve(segments * 4);
      mesh.collision_occluders.reserve(segments);
      mesh.collision_vertices.reserve((segments * 2) + 2 + (segments * 2));

      // Wall
      {
         const uint16 start_bottom_index = vertex_index++;
         const uint16 start_top_index = vertex_index++;

         uint16 last_bottom_index = start_bottom_index;
         uint16 last_top_index = start_top_index;

         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * size.x - float3{0.0f, size.y, 0.0f},
            .surface_index = cylinder_surface_wall,
         });
         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * size.x + float3{0.0f, size.y, 0.0f},
            .surface_index = cylinder_surface_wall,
         });

         for (int i = 1; i < segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endCS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };
            const float3 endLS = endCS * float3{size.x, 0.0f, size.z};

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;
            const uint16 i2 = last_bottom_index;
            const uint16 i3 = last_top_index;

            mesh.collision_vertices.push_back({
               .position = endLS + float3{0.0f, size.y, 0.0f},
               .surface_index = cylinder_surface_wall,
            });
            mesh.collision_vertices.push_back({
               .position = endLS - float3{0.0f, size.y, 0.0f},
               .surface_index = cylinder_surface_wall,
            });

            mesh.collision_triangles.push_back({i0, i1, i2});
            mesh.collision_triangles.push_back({i0, i2, i3});
            mesh.collision_occluders.push_back({i0, i1, i2, i3});

            last_top_index = i0;
            last_bottom_index = i1;
         }

         const uint16 i0 = start_top_index;
         const uint16 i1 = start_bottom_index;
         const uint16 i2 = last_bottom_index;
         const uint16 i3 = last_top_index;

         mesh.collision_triangles.push_back({i0, i1, i2});
         mesh.collision_triangles.push_back({i0, i2, i3});
         mesh.collision_occluders.push_back({i0, i1, i2, i3});
      }

      // Top
      {
         const uint16 centre_index = vertex_index++;
         const uint16 start_index = vertex_index++;
         uint16 last_index = start_index;

         mesh.collision_vertices.push_back({
            .position = float3{0.0f, size.y, 0.0f},
            .surface_index = cylinder_surface_pos_y,
         });
         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * size.x + float3{0.0f, size.y, 0.0f},
            .surface_index = cylinder_surface_pos_y,
         });

         for (int i = 1; i < segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float tri_end = circle_t * pi2;

            const float3 pointLS = {
               cosf(tri_end) * size.x,
               size.y,
               sinf(tri_end) * size.z,
            };

            const uint16 i0 = vertex_index++;
            const uint16 i1 = last_index;
            const uint16 i2 = centre_index;

            mesh.collision_vertices.push_back({
               .position = pointLS,
               .surface_index = cylinder_surface_pos_y,
            });

            mesh.collision_triangles.push_back({i0, i1, i2});

            last_index = i0;
         }

         mesh.collision_triangles.push_back({start_index, last_index, centre_index});
      }

      // Bottom
      {
         const uint16 centre_index = vertex_index++;
         const uint16 start_index = vertex_index++;
         uint16 last_index = start_index;

         mesh.collision_vertices.push_back({
            .position = float3{0.0f, -size.y, 0.0f},
            .surface_index = cylinder_surface_neg_y,
         });
         mesh.collision_vertices.push_back({
            .position = float3{1.0f, 0.0f, 0.0f} * size.x - float3{0.0f, size.y, 0.0f},
            .surface_index = cylinder_surface_neg_y,
         });

         for (int i = 1; i < segments; ++i) {
            const float circle_t = i / static_cast<float>(segments);
            const float tri_end = circle_t * pi2;

            const float3 pointLS = {
               cosf(tri_end) * size.x,
               -size.y,
               sinf(tri_end) * size.z,
            };

            const uint16 i0 = centre_index;
            const uint16 i1 = last_index;
            const uint16 i2 = vertex_index++;

            mesh.collision_vertices.push_back({
               .position = pointLS,
               .surface_index = cylinder_surface_neg_y,
            });

            mesh.collision_triangles.push_back({i0, i1, i2});

            last_index = i2;
         }

         mesh.collision_triangles.push_back({centre_index, last_index, start_index});
      }
   }

   // Snapping Mesh
   {
      uint16 vertex_index = 0;

      mesh.snap_points.reserve(segments * 2);
      mesh.snap_edges.reserve(segments * 3);

      // Wall
      {
         const uint16 start_bottom_index = vertex_index++;
         const uint16 start_top_index = vertex_index++;

         uint16 last_bottom_index = start_bottom_index;
         uint16 last_top_index = start_top_index;

         mesh.snap_points.push_back(
            {float3{1.0f, 0.0f, 0.0f} * size.x - float3{0.0f, size.y, 0.0f}});
         mesh.snap_points.push_back(
            {float3{1.0f, 0.0f, 0.0f} * size.x + float3{0.0f, size.y, 0.0f}});

         for (int i = 1; i < segments; ++i) {

            const float circle_t = i / static_cast<float>(segments);
            const float segment_end = circle_t * pi2;

            const float3 endCS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };
            const float3 endLS = endCS * float3{size.x, 0.0f, size.z};

            const uint16 i0 = vertex_index++;
            const uint16 i1 = vertex_index++;

            mesh.snap_points.push_back({endLS + float3{0.0f, size.y, 0.0f}});
            mesh.snap_points.push_back({endLS - float3{0.0f, size.y, 0.0f}});

            mesh.snap_edges.push_back({i0, i1});
            mesh.snap_edges.push_back({i0, last_top_index});
            mesh.snap_edges.push_back({i1, last_bottom_index});

            last_top_index = i0;
            last_bottom_index = i1;
         }

         mesh.snap_edges.push_back({start_top_index, start_bottom_index});
         mesh.snap_edges.push_back({start_top_index, last_top_index});
         mesh.snap_edges.push_back({start_bottom_index, last_bottom_index});
      }
   }

   return mesh;
}

}