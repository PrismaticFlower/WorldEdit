#include "raycast_animation.hpp"
#include "animation.hpp"
#include "math/intersectors.hpp"

namespace we::world {

auto raycast_position_keys(const float3 ray_origin, const float3 ray_direction,
                           const animation& animation, const quaternion& base_rotation,
                           const float3& base_position,
                           const float visualizer_scale) noexcept
   -> std::optional<int32>
{
   std::optional<int32> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (int32 i = 0; i < std::ssize(animation.position_keys); ++i) {
      float4x4 transform = evaluate_animation(animation, base_rotation, base_position,
                                              animation.position_keys[i].time);

      const float intersection =
         sphIntersect(ray_origin, ray_direction,
                      float3{transform[3].x, transform[3].y, transform[3].z},
                      1.414f * visualizer_scale);

      if (intersection < 0.0f) continue;

      if (intersection < min_distance) {
         hit = i;
         min_distance = intersection;
      }
   }

   return hit;
}

auto raycast_rotation_keys(const float3 ray_origin, const float3 ray_direction,
                           const animation& animation, const quaternion& base_rotation,
                           const float3& base_position,
                           const float visualizer_scale) noexcept
   -> std::optional<int32>
{
   std::optional<int32> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (int32 i = 0; i < std::ssize(animation.rotation_keys); ++i) {
      float4x4 transform = evaluate_animation(animation, base_rotation, base_position,
                                              animation.rotation_keys[i].time);

      const float intersection =
         sphIntersect(ray_origin, ray_direction,
                      float3{transform[3].x, transform[3].y, transform[3].z},
                      visualizer_scale);

      if (intersection < 0.0f) continue;

      if (intersection < min_distance) {
         hit = i;
         min_distance = intersection;
      }
   }

   return hit;
}

}