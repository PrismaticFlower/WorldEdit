#include "mesh_generate.hpp"

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

}