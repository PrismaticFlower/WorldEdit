#include "mesh_generate.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

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

}