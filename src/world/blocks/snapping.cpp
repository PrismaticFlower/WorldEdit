#include "snapping.hpp"
#include "mesh_geometry.hpp"

#include "math/bounding_box.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

namespace {

constexpr float visualizer_size = 0.125f;

void draw_point(tool_visualizers& visualizers, const float3& point,
                const float4& color) noexcept
{
   visualizers.add_octahedron(
      float4x4{
         float4{visualizer_size, 0.0f, 0.0f, 0.0f},
         float4{0.0f, visualizer_size, 0.0f, 0.0f},
         float4{0.0f, 0.0f, visualizer_size, 0.0f},
         float4{point.x, point.y, point.z, 1.0f},
      },
      color);
}

}

auto get_snapped_position(const float3 positionWS, const blocks& blocks,
                          const blocks_snapping_config& config,
                          tool_visualizers& visualizers,
                          const blocks_snapping_visualizer_colors& colors) noexcept -> float3
{
   float3 closest_pointWS;
   float closest_distance = FLT_MAX;

   const int edge_point_count = config.edge_snap_points + 1;
   const float flt_edge_point_count = static_cast<float>(edge_point_count);

   for (uint32 box_index = 0; box_index < blocks.boxes.size(); ++box_index) {
      if (not config.active_layers[blocks.boxes.layer[box_index]]) continue;
      if (blocks.boxes.hidden[box_index]) continue;
      if (blocks.boxes.ids[box_index] == config.filter_id) continue;

      const math::bounding_box bboxWS = {
         .min =
            {
               blocks.boxes.bbox.min_x[box_index],
               blocks.boxes.bbox.min_y[box_index],
               blocks.boxes.bbox.min_z[box_index],
            },

         .max =
            {
               blocks.boxes.bbox.max_x[box_index],
               blocks.boxes.bbox.max_y[box_index],
               blocks.boxes.bbox.max_z[box_index],
            },
      };

      const float3 box_centreOS = (bboxWS.min + bboxWS.max) * 0.5f;
      const float3 box_size = (bboxWS.max - bboxWS.min) * 0.5f;

      const float3 positionAS = positionWS - box_centreOS;
      const float3 distances = abs(positionAS) - box_size;

      const float box_distance =
         length(max(distances, float3{0.0f, 0.0f, 0.0f})) +
         std::min(std::max(std::max(distances.x, distances.y), distances.z), 0.0f);

      if (box_distance > config.snap_radius) continue;

      const block_description_box& box = blocks.boxes.description[box_index];

      const float4x4 scale = {
         {box.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, box.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, box.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(box.rotation);

      float4x4 world_from_object = rotation * scale;
      world_from_object[3] = {box.position, 1.0f};

      std::array<float3, 8> verticesWS;

      for (std::size_t i = 0; i < block_cube_points.size(); ++i) {
         verticesWS[i] = world_from_object * block_cube_points[i];
      }

      for (const float3& vertex_positionWS : verticesWS) {
         const float corner_distance = distance(positionWS, vertex_positionWS);

         if (corner_distance < closest_distance) {
            closest_pointWS = vertex_positionWS;
            closest_distance = corner_distance;
         }

         draw_point(visualizers, vertex_positionWS, colors.corner);
      }

      for (const auto& [i0, i1] : block_cube_edges) {
         for (int i = 1; i < edge_point_count; ++i) {
            const float3 pointWS =
               lerp(verticesWS[i0], verticesWS[i1], i / flt_edge_point_count);
            const float point_distance = distance(positionWS, pointWS);

            if (point_distance < closest_distance) {
               closest_pointWS = pointWS;
               closest_distance = point_distance;
            }

            draw_point(visualizers, pointWS, colors.edge);
         }
      }
   }

   for (uint32 ramp_index = 0; ramp_index < blocks.ramps.size(); ++ramp_index) {
      if (not config.active_layers[blocks.ramps.layer[ramp_index]]) continue;
      if (blocks.ramps.hidden[ramp_index]) continue;
      if (blocks.ramps.ids[ramp_index] == config.filter_id) continue;

      const math::bounding_box bboxWS = {
         .min =
            {
               blocks.ramps.bbox.min_x[ramp_index],
               blocks.ramps.bbox.min_y[ramp_index],
               blocks.ramps.bbox.min_z[ramp_index],
            },

         .max =
            {
               blocks.ramps.bbox.max_x[ramp_index],
               blocks.ramps.bbox.max_y[ramp_index],
               blocks.ramps.bbox.max_z[ramp_index],
            },
      };

      const float3 ramp_centreOS = (bboxWS.min + bboxWS.max) * 0.5f;
      const float3 ramp_size = (bboxWS.max - bboxWS.min) * 0.5f;

      const float3 positionAS = positionWS - ramp_centreOS;
      const float3 distances = abs(positionAS) - ramp_size;

      const float ramp_distance =
         length(max(distances, float3{0.0f, 0.0f, 0.0f})) +
         std::min(std::max(std::max(distances.x, distances.y), distances.z), 0.0f);

      if (ramp_distance > config.snap_radius) continue;

      const block_description_ramp& ramp = blocks.ramps.description[ramp_index];

      const float4x4 scale = {
         {ramp.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, ramp.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, ramp.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(ramp.rotation);

      float4x4 world_from_object = rotation * scale;
      world_from_object[3] = {ramp.position, 1.0f};

      std::array<float3, 8> verticesWS;

      for (std::size_t i = 0; i < block_ramp_points.size(); ++i) {
         verticesWS[i] = world_from_object * block_ramp_points[i];
      }

      for (const float3& vertex_positionWS : verticesWS) {
         const float corner_distance = distance(positionWS, vertex_positionWS);

         if (corner_distance < closest_distance) {
            closest_pointWS = vertex_positionWS;
            closest_distance = corner_distance;
         }

         draw_point(visualizers, vertex_positionWS, colors.corner);
      }

      for (const auto& [i0, i1] : block_ramp_edges) {
         for (int i = 1; i < edge_point_count; ++i) {
            const float3 pointWS =
               lerp(verticesWS[i0], verticesWS[i1], i / flt_edge_point_count);
            const float point_distance = distance(positionWS, pointWS);

            if (point_distance < closest_distance) {
               closest_pointWS = pointWS;
               closest_distance = point_distance;
            }

            draw_point(visualizers, pointWS, colors.edge);
         }
      }
   }

   for (uint32 quad_index = 0; quad_index < blocks.quads.size(); ++quad_index) {
      if (not config.active_layers[blocks.quads.layer[quad_index]]) continue;
      if (blocks.quads.hidden[quad_index]) continue;
      if (blocks.quads.ids[quad_index] == config.filter_id) continue;

      const math::bounding_box bboxWS = {
         .min =
            {
               blocks.quads.bbox.min_x[quad_index],
               blocks.quads.bbox.min_y[quad_index],
               blocks.quads.bbox.min_z[quad_index],
            },

         .max =
            {
               blocks.quads.bbox.max_x[quad_index],
               blocks.quads.bbox.max_y[quad_index],
               blocks.quads.bbox.max_z[quad_index],
            },
      };

      const float3 quad_centreOS = (bboxWS.min + bboxWS.max) * 0.5f;
      const float3 quad_size = (bboxWS.max - bboxWS.min) * 0.5f;

      const float3 positionAS = positionWS - quad_centreOS;
      const float3 distances = abs(positionAS) - quad_size;

      const float quad_distance =
         length(max(distances, float3{0.0f, 0.0f, 0.0f})) +
         std::min(std::max(std::max(distances.x, distances.y), distances.z), 0.0f);

      if (quad_distance > config.snap_radius) continue;

      const block_description_quad& quad = blocks.quads.description[quad_index];

      for (const float3& vertex_positionWS : quad.vertices) {
         const float corner_distance = distance(positionWS, vertex_positionWS);

         if (corner_distance < closest_distance) {
            closest_pointWS = vertex_positionWS;
            closest_distance = corner_distance;
         }

         draw_point(visualizers, vertex_positionWS, colors.corner);
      }

      for (const auto& [i0, i1] : std::initializer_list{
              std::array<int8, 2>{0, 1},
              std::array<int8, 2>{1, 2},
              std::array<int8, 2>{2, 3},
              std::array<int8, 2>{3, 0},
           }) {
         for (int i = 1; i < edge_point_count; ++i) {
            const float3 pointWS =
               lerp(quad.vertices[i0], quad.vertices[i1], i / flt_edge_point_count);
            const float point_distance = distance(positionWS, pointWS);

            if (point_distance < closest_distance) {
               closest_pointWS = pointWS;
               closest_distance = point_distance;
            }

            draw_point(visualizers, pointWS, colors.edge);
         }
      }
   }

   for (uint32 cylinder_index = 0; cylinder_index < blocks.cylinders.size();
        ++cylinder_index) {
      if (not config.active_layers[blocks.cylinders.layer[cylinder_index]])
         continue;
      if (blocks.cylinders.hidden[cylinder_index]) continue;
      if (blocks.cylinders.ids[cylinder_index] == config.filter_id) continue;

      const math::bounding_box bboxWS = {
         .min =
            {
               blocks.cylinders.bbox.min_x[cylinder_index],
               blocks.cylinders.bbox.min_y[cylinder_index],
               blocks.cylinders.bbox.min_z[cylinder_index],
            },

         .max =
            {
               blocks.cylinders.bbox.max_x[cylinder_index],
               blocks.cylinders.bbox.max_y[cylinder_index],
               blocks.cylinders.bbox.max_z[cylinder_index],
            },
      };

      const float3 cylinder_centreOS = (bboxWS.min + bboxWS.max) * 0.5f;
      const float3 cylinder_size = (bboxWS.max - bboxWS.min) * 0.5f;

      const float3 positionAS = positionWS - cylinder_centreOS;
      const float3 distances = abs(positionAS) - cylinder_size;

      const float cylinder_distance =
         length(max(distances, float3{0.0f, 0.0f, 0.0f})) +
         std::min(std::max(std::max(distances.x, distances.y), distances.z), 0.0f);

      if (cylinder_distance > config.snap_radius) continue;

      const block_description_cylinder& cylinder =
         blocks.cylinders.description[cylinder_index];

      const float4x4 scale = {
         {cylinder.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, cylinder.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, cylinder.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(cylinder.rotation);

      float4x4 world_from_object = rotation * scale;
      world_from_object[3] = {cylinder.position, 1.0f};

      std::array<float3, 64> verticesWS;

      for (std::size_t i = 0; i < block_cylinder_points.size(); ++i) {
         verticesWS[i] = world_from_object * block_cylinder_points[i];
      }

      for (const float3& vertex_positionWS : verticesWS) {
         const float corner_distance = distance(positionWS, vertex_positionWS);

         if (corner_distance < closest_distance) {
            closest_pointWS = vertex_positionWS;
            closest_distance = corner_distance;
         }

         draw_point(visualizers, vertex_positionWS, colors.corner);
      }

      for (const auto& [i0, i1] : block_cylinder_edges) {
         for (int i = 1; i < edge_point_count; ++i) {
            const float3 pointWS =
               lerp(verticesWS[i0], verticesWS[i1], i / flt_edge_point_count);
            const float point_distance = distance(positionWS, pointWS);

            if (point_distance < closest_distance) {
               closest_pointWS = pointWS;
               closest_distance = point_distance;
            }

            draw_point(visualizers, pointWS, colors.edge);
         }
      }
   }

   float3 new_positionWS;

   if (closest_distance > 0.0f and closest_distance <= config.snap_radius) {
      const float3 snap_directionWS = normalize(closest_pointWS - positionWS);

      new_positionWS = positionWS + snap_directionWS * closest_distance;
   }
   else {
      new_positionWS = positionWS;
   }

   if (closest_distance <= config.snap_radius) {
      visualizers.add_octahedron(
         float4x4{
            float4{visualizer_size * 2.0f, 0.0f, 0.0f, 0.0f},
            float4{0.0f, visualizer_size * 2.0f, 0.0f, 0.0f},
            float4{0.0f, 0.0f, visualizer_size * 2.0f, 0.0f},
            float4{closest_pointWS.x, closest_pointWS.y, closest_pointWS.z, 1.0f},
         },
         colors.snapped);
   }

   return new_positionWS;
}

}