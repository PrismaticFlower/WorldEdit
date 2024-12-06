#include "snapping.hpp"
#include "../object_class.hpp"

#include "math/intersectors.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <optional>

namespace we::world {

namespace {

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

}

auto get_snapped_position(const snapping_entity& snapping,
                          const std::span<const object> world_objects,
                          const float snap_radius, const snapping_flags flags,
                          const active_layers active_layers,
                          const object_class_library& object_classes) noexcept -> float3
{
   const float3 ray_originWS =
      snapping.positionWS + (snapping.bboxOS.min + snapping.bboxOS.max) * 0.5f;

   const float3 x_dirWS = normalize(snapping.rotation * float3{1.0f, 0.0f, 0.0f});
   const float3 y_dirWS = normalize(snapping.rotation * float3{0.0f, 1.0f, 0.0f});
   const float3 z_dirWS = normalize(snapping.rotation * float3{0.0f, 0.0f, 1.0f});

   float closest_hit = FLT_MAX;
   float3 closest_hit_dirWS = {};
   std::optional<uint32> closest_hit_object_index;

   float closest_object_distance = FLT_MAX;
   std::optional<uint32> closest_object_index;

   for (uint32 object_index = 0; object_index < world_objects.size(); ++object_index) {
      const object& object = world_objects[object_index];

      if (not active_layers[object.layer]) continue;
      if (object.hidden) continue;

      const assets::msh::flat_model& model =
         *object_classes[object.class_handle].model;

      // Trace rays for surface snapping.
      if (flags.snap_to_surfaces) {
         const quaternion object_from_world = conjugate(object.rotation);
         const float3 position_offsetOS = object_from_world * -object.position;

         const float3 ray_originOS =
            object_from_world * ray_originWS + position_offsetOS;

         const float3 snap_x_directionOS = normalize(object_from_world * x_dirWS);
         const float3 snap_y_directionOS = normalize(object_from_world * y_dirWS);
         const float3 snap_z_directionOS = normalize(object_from_world * z_dirWS);

         struct ray_data {
            float3 directionOS;
            float bbox_length;
            float3 directionWS;
         };

         const std::array<ray_data, 6> rays = {{
            {snap_x_directionOS, std::abs(snapping.bboxOS.max.x), x_dirWS},
            {snap_y_directionOS, std::abs(snapping.bboxOS.max.y), y_dirWS},
            {snap_z_directionOS, std::abs(snapping.bboxOS.max.z), z_dirWS},
            {-snap_x_directionOS, std::abs(snapping.bboxOS.min.x), -x_dirWS},
            {-snap_y_directionOS, std::abs(snapping.bboxOS.min.y), -y_dirWS},
            {-snap_z_directionOS, std::abs(snapping.bboxOS.min.z), -z_dirWS},
         }};

         for (const ray_data& ray : rays) {
            if (float bbox_hit = FLT_MAX;
                intersect_aabb(ray_originOS, 1.0f / ray.directionOS,
                               model.bounding_box, closest_hit, bbox_hit)) {
               if (auto hit = model.bvh.query(ray_originOS, ray.directionOS);
                   hit and hit->distance - ray.bbox_length < closest_hit) {
                  closest_hit = hit->distance - ray.bbox_length;
                  closest_hit_dirWS = ray.directionWS;
                  closest_hit_object_index = object_index;
               }
            }
         }
      }

      // Calculate nearest box for point snapping.
      {
         const math::bounding_box& bboxOS = model.bounding_box;

         const float3 positionOS =
            conjugate(object.rotation) * (snapping.positionWS - object.position);
         const float3 box_centreOS = (bboxOS.min + bboxOS.max) * 0.5f;
         const float3 box_size = (bboxOS.max - bboxOS.min) * 0.5f;

         const float3 positionAS = positionOS - box_centreOS;
         const float3 distances = abs(positionAS) - box_size;

         const float distance =
            std::max(std::max(distances.x, distances.y), distances.z);

         if (distance >= 0.0f and distance < closest_object_distance) {
            closest_object_distance = distance;
            closest_object_index = object_index;
         }
      }
   }

   if (closest_hit <= snap_radius) {
      closest_object_index = closest_hit_object_index;
   }

   if (not closest_object_index) return snapping.positionWS;

   const object& closest_object = world_objects[*closest_object_index];

   const std::array<float3, 8> snapping_object_cornersWS =
      get_snapping_corners(snapping.rotation, snapping.positionWS, snapping.bboxOS);
   const std::array<float3, 8> closest_object_cornersWS =
      get_snapping_corners(closest_object.rotation, closest_object.position,
                           object_classes[closest_object.class_handle].model->bounding_box);

   float3 closest_pointWS;
   float closest_distance = FLT_MAX;
   float3 snapping_pointWS;

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
      }
   }

   const std::array<float3, 12> snapping_object_edgesWS =
      get_snapping_edge_midpoints(snapping_object_cornersWS);
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
      }
   }

   if (flags.snap_to_face_midpoints) {
      const std::array<float3, 2> snapping_object_facesWS =
         get_snapping_face_midpoints(snapping_object_cornersWS);
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
      }

      for (const float3& pointWS : closest_object_facesWS) {
         for (uint32 i = 0; i < snapping_object_edgesWS.size(); ++i) {
            const float point_distance =
               distance(pointWS, snapping_object_edgesWS[i]);

            if (point_distance < closest_distance) {
               closest_pointWS = pointWS;
               closest_distance = point_distance;
               snapping_pointWS = snapping_object_edgesWS[i];
            }
         }
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

   if (closest_distance <= snap_radius) {
      const float3 snap_directionWS = normalize(closest_pointWS - snapping_pointWS);

      return snapping.positionWS + snap_directionWS * closest_distance;
   }
   else if (closest_hit <= snap_radius) {
      return snapping.positionWS + closest_hit_dirWS * closest_hit;
   }
   else {
      return snapping.positionWS;
   }
}

auto get_snapped_position(const object& snapping_object, const float3 snapping_positionWS,
                          const std::span<const object> world_objects,
                          const float snap_radius, const snapping_flags flags,
                          const active_layers active_layers,
                          const object_class_library& object_classes) noexcept -> float3
{
   return get_snapped_position(
      snapping_entity{.rotation = snapping_object.rotation,
                      .positionWS = snapping_positionWS,
                      .bboxOS =
                         object_classes[snapping_object.class_handle].model->bounding_box},
      world_objects, snap_radius, flags, active_layers, object_classes);
}

}