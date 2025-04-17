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