#include "grounding.hpp"
#include "../object_class.hpp"
#include "raycast.hpp"

namespace we::world {

auto ground_object(const object& obj, const world& world,
                   const object_class_library& object_classes,
                   const active_layers active_layers,
                   const terrain_collision& terrain_collision) -> std::optional<float3>
{
   const math::bounding_box bbox =
      obj.rotation * object_classes[obj.class_name].model->bounding_box + obj.position;

   float hit_distance = std::numeric_limits<float>::max();

   const float3 ray_origin = {obj.position.x, bbox.min.y, obj.position.z};

   if (std::optional<raycast_result<object>> hit =
          raycast(ray_origin, {0.0f, -1.0f, 0.0f}, active_layers, world.objects,
                  object_classes, obj.id);
       hit) {
      if (hit->distance < hit_distance) {
         hit_distance = hit->distance;
      }
   }

   if (auto hit = terrain_collision.raycast(ray_origin, {0.0f, -1.0f, 0.0f}); hit) {
      if (hit->distance < hit_distance) {
         hit_distance = hit->distance;
      }
   }

   // Try "digging" the object out of the ground.
   if (hit_distance == std::numeric_limits<float>::max()) {
      if (std::optional<raycast_result<object>> hit =
             raycast(ray_origin, {0.0f, 1.0f, 0.0f}, active_layers,
                     world.objects, object_classes, obj.id);
          hit) {
         if (hit->distance < hit_distance) {
            hit_distance = hit->distance;
         }
      }

      if (auto hit = terrain_collision.raycast(ray_origin, {0.0f, 1.0f, 0.0f}); hit) {
         if (hit->distance < hit_distance) {
            hit_distance = hit->distance;
         }
      }

      // Make sure we're not about to "ceiling" the object instead.
      if (hit_distance < std::abs(bbox.max.y - bbox.min.y)) {
         hit_distance = -hit_distance;
      }
      else {
         hit_distance = std::numeric_limits<float>::max();
      }
   }

   if (hit_distance != std::numeric_limits<float>::max()) {
      const float3 new_position = {obj.position.x, obj.position.y - hit_distance,
                                   obj.position.z};

      if (new_position != obj.position) return new_position;
   }

   return std::nullopt;
}

}