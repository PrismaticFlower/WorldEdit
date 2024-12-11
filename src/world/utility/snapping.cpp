#include "snapping.hpp"
#include "../object_class.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

namespace {

constexpr float visualizer_size = 0.125f;

auto get_snapping_corners(const quaternion& rotation, const float3 positionWS,
                          const math::bounding_box& bboxOS) -> std::array<float3, 8>
{
   std::array<float3, 8> cornersWS = math::to_corners(bboxOS);

   for (float3& v : cornersWS) v = rotation * v + positionWS;

   return cornersWS;
}

auto get_snapping_edge_midpoints(const std::array<float3, 8>& corners)
   -> std::array<float3, 12>
{
   return {
      (corners[0] + corners[1]) * 0.5f, (corners[1] + corners[2]) * 0.5f,
      (corners[2] + corners[3]) * 0.5f, (corners[3] + corners[0]) * 0.5f,

      (corners[4] + corners[5]) * 0.5f, (corners[5] + corners[6]) * 0.5f,
      (corners[6] + corners[7]) * 0.5f, (corners[7] + corners[4]) * 0.5f,

      (corners[0] + corners[4]) * 0.5f, (corners[1] + corners[5]) * 0.5f,
      (corners[2] + corners[6]) * 0.5f, (corners[3] + corners[7]) * 0.5f,
   };
}

auto get_snapping_face_midpoints(const std::array<float3, 8>& corners)
   -> std::array<float3, 2>
{
   return {
      (corners[0] + corners[1] + corners[2] + corners[3]) * 0.25f, // y+
      (corners[4] + corners[5] + corners[6] + corners[7]) * 0.25f, // y-
   };
}

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

auto get_snapped_position(const snapping_entity& snapping,
                          const std::span<const object> world_objects,
                          const float snap_radius, const snapping_flags flags,
                          const active_layers active_layers,
                          const object_class_library& object_classes,
                          tool_visualizers& visualizers,
                          const snapping_visualizer_colors& colors) noexcept -> float3
{
   const float cull_distance =
      distance(snapping.bboxOS.min, snapping.bboxOS.max) + snap_radius;

   const std::array<float3, 8> snapping_object_cornersWS =
      get_snapping_corners(snapping.rotation, snapping.positionWS, snapping.bboxOS);
   const std::array<float3, 12> snapping_object_edgesWS =
      get_snapping_edge_midpoints(snapping_object_cornersWS);
   const std::array<float3, 2> snapping_object_facesWS =
      get_snapping_face_midpoints(snapping_object_cornersWS);

   float3 closest_pointWS;
   float closest_distance = FLT_MAX;
   float3 snapping_pointWS;

   for (uint32 object_index = 0; object_index < world_objects.size(); ++object_index) {
      const object& object = world_objects[object_index];

      if (not active_layers[object.layer]) continue;
      if (object.hidden) continue;

      const assets::msh::flat_model& model =
         *object_classes[object.class_handle].model;

      const math::bounding_box& bboxOS = model.bounding_box;

      const float3 positionOS =
         conjugate(object.rotation) * (snapping.positionWS - object.position);
      const float3 box_centreOS = (bboxOS.min + bboxOS.max) * 0.5f;
      const float3 box_size = (bboxOS.max - bboxOS.min) * 0.5f;

      const float3 positionAS = positionOS - box_centreOS;
      const float3 distances = abs(positionAS) - box_size;

      const float box_distance = length(
         max(distances, float3{0.0f, 0.0f, 0.0f}) +
         std::min(std::max(std::max(distances.x, distances.y), distances.z), 0.0f));

      if (box_distance > cull_distance) continue;

      const std::array<float3, 8> closest_object_cornersWS =
         get_snapping_corners(object.rotation, object.position,
                              object_classes[object.class_handle].model->bounding_box);

      if (flags.snap_to_corners) {
         for (const float3& cornerWS : closest_object_cornersWS) {
            for (uint32 i = 0; i < snapping_object_cornersWS.size(); ++i) {
               const float corner_distance =
                  distance(cornerWS, snapping_object_cornersWS[i]);

               if (corner_distance < closest_distance) {
                  closest_pointWS = cornerWS;
                  closest_distance = corner_distance;
                  snapping_pointWS = snapping_object_cornersWS[i];
               }
            }

            draw_point(visualizers, cornerWS, colors.corner);
         }
      }

      const std::array<float3, 12> closest_object_edgesWS =
         get_snapping_edge_midpoints(closest_object_cornersWS);

      if (flags.snap_to_edge_midpoints) {
         for (const float3& pointWS : closest_object_edgesWS) {
            for (uint32 i = 0; i < snapping_object_edgesWS.size(); ++i) {
               const float point_distance =
                  distance(pointWS, snapping_object_edgesWS[i]);

               if (point_distance < closest_distance) {
                  closest_pointWS = pointWS;
                  closest_distance = point_distance;
                  snapping_pointWS = snapping_object_edgesWS[i];
               }
            }

            draw_point(visualizers, pointWS, colors.edge);
         }
      }

      if (flags.snap_to_face_midpoints) {
         const std::array<float3, 2> closest_object_facesWS =
            get_snapping_face_midpoints(closest_object_cornersWS);

         for (const float3& pointWS : closest_object_facesWS) {
            for (uint32 i = 0; i < snapping_object_facesWS.size(); ++i) {
               const float point_distance =
                  distance(pointWS, snapping_object_facesWS[i]);

               if (point_distance < closest_distance) {
                  closest_pointWS = pointWS;
                  closest_distance = point_distance;
                  snapping_pointWS = snapping_object_facesWS[i];
               }
            }

            for (uint32 i = 0; i < snapping_object_edgesWS.size(); ++i) {
               const float point_distance =
                  distance(pointWS, snapping_object_edgesWS[i]);

               if (point_distance < closest_distance) {
                  closest_pointWS = pointWS;
                  closest_distance = point_distance;
                  snapping_pointWS = snapping_object_edgesWS[i];
               }
            }

            draw_point(visualizers, pointWS, colors.edge);
         }

         for (const float3& pointWS : closest_object_edgesWS) {
            for (uint32 i = 0; i < snapping_object_facesWS.size(); ++i) {
               const float point_distance =
                  distance(pointWS, snapping_object_facesWS[i]);

               if (point_distance < closest_distance) {
                  closest_pointWS = pointWS;
                  closest_distance = point_distance;
                  snapping_pointWS = snapping_object_facesWS[i];
               }
            }
         }
      }
   }

   float3 new_positionWS;

   if (closest_distance > 0.0f and closest_distance <= snap_radius) {
      const float3 snap_directionWS = normalize(closest_pointWS - snapping_pointWS);

      new_positionWS = snapping.positionWS + snap_directionWS * closest_distance;
   }
   else {
      new_positionWS = snapping.positionWS;
   }

   const std::array<float3, 8> snapped_object_cornersWS =
      get_snapping_corners(snapping.rotation, new_positionWS, snapping.bboxOS);

   if (flags.snap_to_corners) {
      for (const float3& pointWS : snapped_object_cornersWS) {
         draw_point(visualizers, pointWS, colors.corner);
      }
   }

   if (flags.snap_to_edge_midpoints) {
      for (const float3& pointWS :
           get_snapping_edge_midpoints(snapped_object_cornersWS)) {
         draw_point(visualizers, pointWS, colors.edge);
      }
   }

   if (flags.snap_to_face_midpoints) {
      for (const float3& pointWS :
           get_snapping_face_midpoints(snapped_object_cornersWS)) {
         draw_point(visualizers, pointWS, colors.face);
      }
   }

   if (closest_distance <= snap_radius) {
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

auto get_snapped_position(const object& snapping_object, const float3 snapping_positionWS,
                          const std::span<const object> world_objects,
                          const float snap_radius, const snapping_flags flags,
                          const active_layers active_layers,
                          const object_class_library& object_classes,
                          tool_visualizers& visualizers,
                          const snapping_visualizer_colors& colors) noexcept -> float3
{
   return get_snapped_position(snapping_entity{.rotation = snapping_object.rotation,
                                               .positionWS = snapping_positionWS,
                                               .bboxOS =
                                                  object_classes[snapping_object.class_handle]
                                                     .model->bounding_box},
                               world_objects, snap_radius, flags, active_layers,
                               object_classes, visualizers, colors);
}

auto get_snapped_position_filtered(
   const snapping_entity& snapping, const std::span<const object> world_objects,
   const selection& selection, const float snap_radius,
   const snapping_flags flags, const active_layers active_layers,
   const object_class_library& object_classes, tool_visualizers& visualizers,
   const snapping_visualizer_colors& colors) noexcept -> float3
{
   const float cull_distance =
      distance(snapping.bboxOS.min, snapping.bboxOS.max) + snap_radius;

   const std::array<float3, 8> snapping_object_cornersWS =
      get_snapping_corners(snapping.rotation, snapping.positionWS, snapping.bboxOS);
   const std::array<float3, 12> snapping_object_edgesWS =
      get_snapping_edge_midpoints(snapping_object_cornersWS);
   const std::array<float3, 2> snapping_object_facesWS =
      get_snapping_face_midpoints(snapping_object_cornersWS);

   float3 closest_pointWS;
   float closest_distance = FLT_MAX;
   float3 snapping_pointWS;

   for (uint32 object_index = 0; object_index < world_objects.size(); ++object_index) {
      const object& object = world_objects[object_index];

      if (not active_layers[object.layer]) continue;
      if (object.hidden) continue;
      if (is_selected(object.id, selection)) continue;

      const assets::msh::flat_model& model =
         *object_classes[object.class_handle].model;

      const math::bounding_box& bboxOS = model.bounding_box;

      const float3 positionOS =
         conjugate(object.rotation) * (snapping.positionWS - object.position);
      const float3 box_centreOS = (bboxOS.min + bboxOS.max) * 0.5f;
      const float3 box_size = (bboxOS.max - bboxOS.min) * 0.5f;

      const float3 positionAS = positionOS - box_centreOS;
      const float3 distances = abs(positionAS) - box_size;

      const float box_distance = length(
         max(distances, float3{0.0f, 0.0f, 0.0f}) +
         std::min(std::max(std::max(distances.x, distances.y), distances.z), 0.0f));

      if (box_distance > cull_distance) continue;

      const std::array<float3, 8> closest_object_cornersWS =
         get_snapping_corners(object.rotation, object.position,
                              object_classes[object.class_handle].model->bounding_box);

      if (flags.snap_to_corners) {
         for (const float3& cornerWS : closest_object_cornersWS) {
            for (uint32 i = 0; i < snapping_object_cornersWS.size(); ++i) {
               const float corner_distance =
                  distance(cornerWS, snapping_object_cornersWS[i]);

               if (corner_distance < closest_distance) {
                  closest_pointWS = cornerWS;
                  closest_distance = corner_distance;
                  snapping_pointWS = snapping_object_cornersWS[i];
               }
            }

            draw_point(visualizers, cornerWS, colors.corner);
         }
      }

      const std::array<float3, 12> closest_object_edgesWS =
         get_snapping_edge_midpoints(closest_object_cornersWS);

      if (flags.snap_to_edge_midpoints) {
         for (const float3& pointWS : closest_object_edgesWS) {
            for (uint32 i = 0; i < snapping_object_edgesWS.size(); ++i) {
               const float point_distance =
                  distance(pointWS, snapping_object_edgesWS[i]);

               if (point_distance < closest_distance) {
                  closest_pointWS = pointWS;
                  closest_distance = point_distance;
                  snapping_pointWS = snapping_object_edgesWS[i];
               }
            }

            draw_point(visualizers, pointWS, colors.edge);
         }
      }

      if (flags.snap_to_face_midpoints) {
         const std::array<float3, 2> closest_object_facesWS =
            get_snapping_face_midpoints(closest_object_cornersWS);

         for (const float3& pointWS : closest_object_facesWS) {
            for (uint32 i = 0; i < snapping_object_facesWS.size(); ++i) {
               const float point_distance =
                  distance(pointWS, snapping_object_facesWS[i]);

               if (point_distance < closest_distance) {
                  closest_pointWS = pointWS;
                  closest_distance = point_distance;
                  snapping_pointWS = snapping_object_facesWS[i];
               }
            }

            for (uint32 i = 0; i < snapping_object_edgesWS.size(); ++i) {
               const float point_distance =
                  distance(pointWS, snapping_object_edgesWS[i]);

               if (point_distance < closest_distance) {
                  closest_pointWS = pointWS;
                  closest_distance = point_distance;
                  snapping_pointWS = snapping_object_edgesWS[i];
               }
            }

            draw_point(visualizers, pointWS, colors.edge);
         }

         for (const float3& pointWS : closest_object_edgesWS) {
            for (uint32 i = 0; i < snapping_object_facesWS.size(); ++i) {
               const float point_distance =
                  distance(pointWS, snapping_object_facesWS[i]);

               if (point_distance < closest_distance) {
                  closest_pointWS = pointWS;
                  closest_distance = point_distance;
                  snapping_pointWS = snapping_object_facesWS[i];
               }
            }
         }
      }
   }

   float3 new_positionWS;

   if (closest_distance > 0.0f and closest_distance <= snap_radius) {
      const float3 snap_directionWS = normalize(closest_pointWS - snapping_pointWS);

      new_positionWS = snapping.positionWS + snap_directionWS * closest_distance;
   }
   else {
      new_positionWS = snapping.positionWS;
   }

   const std::array<float3, 8> snapped_object_cornersWS =
      get_snapping_corners(snapping.rotation, new_positionWS, snapping.bboxOS);

   if (flags.snap_to_corners) {
      for (const float3& pointWS : snapped_object_cornersWS) {
         draw_point(visualizers, pointWS, colors.corner);
      }
   }

   if (flags.snap_to_edge_midpoints) {
      for (const float3& pointWS :
           get_snapping_edge_midpoints(snapped_object_cornersWS)) {
         draw_point(visualizers, pointWS, colors.edge);
      }
   }

   if (flags.snap_to_face_midpoints) {
      for (const float3& pointWS :
           get_snapping_face_midpoints(snapped_object_cornersWS)) {
         draw_point(visualizers, pointWS, colors.face);
      }
   }

   if (closest_distance <= snap_radius) {
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