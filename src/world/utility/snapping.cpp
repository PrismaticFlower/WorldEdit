#include "snapping.hpp"
#include "../object_class.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

namespace {

auto get_transformed_corners(const object& object, const float3 object_position,
                             const object_class& object_class)
   -> std::array<float3, 8>
{
   std::array<float3, 8> snapping_corners =
      math::to_corners(object_class.model->bounding_box);

   for (auto& v : snapping_corners) {
      v = object.rotation * v + object_position;
   }

   return snapping_corners;
}

auto get_face_midpoints(const std::array<float3, 8>& corners) -> std::array<float3, 6>
{
   constexpr std::array<std::array<uint8, 4>, 6> indices{{
      {0, 1, 2, 3}, // Y+
      {4, 5, 6, 7}, // Y-
      {0, 3, 4, 7}, // X+
      {1, 2, 5, 6}, // X-
      {0, 1, 4, 5}, // Z+
      {2, 3, 6, 7}, // Z-
   }};

   std::array<float3, 6> face_midpoints;

   for (std::size_t i = 0; i < 6; ++i) {
      float3 position = {0.0f, 0.0f, 0.0f};

      for (uint8 index : indices[i]) {
         position += corners[index];
      }

      face_midpoints[i] = position / 4.0f;
   }

   return face_midpoints;
}

auto get_snapping_points(const object& object, const float3 object_position,
                         const object_class& object_class) -> std::array<float3, 18>
{
   const std::array<float3, 8> corners =
      math::to_corners(object_class.model->bounding_box);

   std::array<float3, 18> points;

   for (std::size_t i = 0; i < corners.size(); ++i) {
      points[i] = corners[i];
   }

   // edges
   points[8] = (corners[0] + corners[1]) / 2.0f;
   points[9] = (corners[1] + corners[2]) / 2.0f;
   points[10] = (corners[2] + corners[3]) / 2.0f;
   points[11] = (corners[3] + corners[0]) / 2.0f;

   points[12] = (corners[4] + corners[5]) / 2.0f;
   points[13] = (corners[5] + corners[6]) / 2.0f;
   points[14] = (corners[6] + corners[7]) / 2.0f;
   points[15] = (corners[7] + corners[4]) / 2.0f;

   points[16] = (corners[0] + corners[1] + corners[2] + corners[3]) / 4.0f;
   points[16] = (corners[4] + corners[5] + corners[6] + corners[7]) / 4.0f;

   for (auto& v : points) {
      v = object.rotation * v + object_position;
   }

   return points;
}

}

auto get_snapped_position(const object& snapping_object, const float3 snapping_position,
                          const std::span<const object> world_objects,
                          const float snap_radius,
                          const object_class_library& object_classes)
   -> std::optional<float3>
{
   const std::array<float3, 18> snapping_corners =
      get_snapping_points(snapping_object, snapping_position,
                          object_classes[snapping_object.class_handle]);

   float3 closest_corner;
   float closest_distance = FLT_MAX;
   uint32 closest_index = 0;

   for (const auto& object : world_objects) {
      const std::array<float3, 18> object_corners =
         get_snapping_points(object, object.position,
                             object_classes[object.class_handle]);

      for (const auto& corner : object_corners) {
         for (uint32 i = 0; i < snapping_corners.size(); ++i) {
            const float corner_distance = distance(corner, snapping_corners[i]);

            if (corner_distance < closest_distance) {
               closest_corner = corner;
               closest_distance = corner_distance;
               closest_index = i;
            }
         }
      }
   }

   if (closest_distance > snap_radius) return std::nullopt;

   float3 direction = normalize(closest_corner - snapping_corners[closest_index]);

   return snapping_position + direction * closest_distance;
}

auto get_snapped_position(const float3 snapping_position,
                          const std::span<const object> world_objects,
                          const float snap_radius,
                          const object_class_library& object_classes)
   -> std::optional<float3>
{
   float3 closest_position;
   float closest_distance = FLT_MAX;

   for (const auto& object : world_objects) {
      const std::array<float3, 6> object_face_midpoints = get_face_midpoints(
         get_transformed_corners(object, object.position,
                                 object_classes[object.class_handle]));

      for (const auto& corner : object_face_midpoints) {
         const float corner_distance = distance(corner, snapping_position);

         if (corner_distance < closest_distance) {
            closest_position = corner;
            closest_distance = corner_distance;
         }
      }
   }

   if (closest_distance > snap_radius) return std::nullopt;

   float3 direction = normalize(closest_position - snapping_position);

   return snapping_position + direction * closest_distance;
}

}