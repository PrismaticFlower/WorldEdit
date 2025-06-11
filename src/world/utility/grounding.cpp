#include "grounding.hpp"
#include "raycast.hpp"
#include "raycast_terrain.hpp"

#include "../blocks/utility/raycast.hpp"
#include "../object_class.hpp"

#include "math/vector_funcs.hpp"

namespace we::world {

namespace {

auto ground_bbox(const float3 position, const math::bounding_box bbox,
                 const world& world, const object_class_library& object_classes,
                 const active_layers active_layers,
                 std::optional<object_id> ignore_object = std::nullopt) noexcept
   -> std::optional<float3>
{
   float hit_distance = std::numeric_limits<float>::max();

   const float3 ray_origin = {position.x, bbox.min.y, position.z};

   const auto raycast_filter = [ignore_object](const object& object) noexcept {
      return object.id != ignore_object;
   };

   if (std::optional<raycast_result<object>> hit =
          raycast(ray_origin, {0.0f, -1.0f, 0.0f}, active_layers, world.objects,
                  object_classes, raycast_filter);
       hit) {
      if (hit->distance < hit_distance) {
         hit_distance = hit->distance;
      }
   }

   if (std::optional<raycast_block_result> hit =
          raycast(ray_origin, {0.0f, -1.0f, 0.0f}, active_layers, world.blocks);
       hit) {
      if (hit->distance < hit_distance) {
         hit_distance = hit->distance;
      }
   }

   if (auto hit = raycast(ray_origin, {0.0f, -1.0f, 0.0f}, world.terrain); hit) {
      if (*hit < hit_distance) hit_distance = *hit;
   }

   // Try "digging" the BBOX out of the ground.
   if (hit_distance == std::numeric_limits<float>::max()) {
      if (std::optional<raycast_result<object>> hit =
             raycast(ray_origin, {0.0f, 1.0f, 0.0f}, active_layers,
                     world.objects, object_classes, raycast_filter);
          hit) {
         if (hit->distance < hit_distance) {
            hit_distance = hit->distance;
         }
      }

      if (std::optional<raycast_block_result> hit =
             raycast(ray_origin, {0.0f, 1.0f, 0.0f}, active_layers, world.blocks);
          hit) {
         if (hit->distance < hit_distance) {
            hit_distance = hit->distance;
         }
      }

      if (auto hit = raycast(ray_origin, {0.0f, 1.0f, 0.0f}, world.terrain); hit) {
         if (*hit < hit_distance) hit_distance = *hit;
      }

      // Make sure we're not about to "ceiling" the BBOX instead.
      if (hit_distance < std::abs(bbox.max.y - bbox.min.y)) {
         hit_distance = -hit_distance;
      }
      else {
         hit_distance = std::numeric_limits<float>::max();
      }
   }

   if (hit_distance != std::numeric_limits<float>::max()) {
      const float3 new_position = {position.x, position.y - hit_distance,
                                   position.z};

      if (new_position != position) return new_position;
   }

   return std::nullopt;
}

auto ground_region_box(const float3 size, const quaternion rotation,
                       const float3 position, const world& world,
                       const object_class_library& object_classes,
                       const active_layers active_layers) noexcept
   -> std::optional<float3>
{
   return ground_bbox(position, rotation * math::bounding_box{-size, size} + position,
                      world, object_classes, active_layers);
}

auto ground_region_cylinder(const float3 size, const quaternion rotation,
                            const float3 position, const world& world,
                            const object_class_library& object_classes,
                            const active_layers active_layers) noexcept
   -> std::optional<float3>
{
   const float radius = length(float2{size.x, size.z});
   const float3 true_size = {radius, size.y, radius};

   return ground_bbox(position,
                      rotation * math::bounding_box{-true_size, true_size} + position,
                      world, object_classes, active_layers);
}

}

auto ground_object(const object& object, const world& world,
                   const object_class_library& object_classes,
                   const active_layers active_layers) noexcept -> std::optional<float3>
{
   return ground_bbox(object.position,
                      object.rotation *
                            object_classes[object.class_handle].model->bounding_box +
                         object.position,
                      world, object_classes, active_layers);
}

auto ground_light(const light& light, const world& world,
                  const object_class_library& object_classes,
                  const active_layers active_layers) noexcept -> std::optional<float3>
{
   switch (light.light_type) {
   case light_type::directional:
   case light_type::point:
   case light_type::directional_region_sphere:
      return ground_point(light.position, world, object_classes, active_layers);
   case light_type::spot: {
      const float half_range = light.range / 2.0f;
      const float outer_cone_radius = half_range * std::tan(light.outer_cone_angle);
      const float inner_cone_radius = half_range * std::tan(light.inner_cone_angle);
      const float radius = std::min(outer_cone_radius, inner_cone_radius);

      const math::bounding_box bbox =
         light.rotation * math::bounding_box{.min = {-radius, -radius, 0.0f},
                                             .max = {radius, radius, light.range}} +
         light.position;

      return ground_bbox(light.position, bbox, world, object_classes, active_layers);
   }
   case light_type::directional_region_box:
      return ground_region_box(light.region_size, light.region_rotation,
                               light.position, world, object_classes, active_layers);
   case light_type::directional_region_cylinder:
      return ground_region_cylinder(light.region_size, light.region_rotation,
                                    light.position, world, object_classes,
                                    active_layers);
   default:
      return std::nullopt;
   }
}

auto ground_region(const region& region, const world& world,
                   const object_class_library& object_classes,
                   const active_layers active_layers) noexcept -> std::optional<float3>
{
   switch (region.shape) {
   case region_shape::box:
      return ground_region_box(region.size, region.rotation, region.position,
                               world, object_classes, active_layers);
   case region_shape::sphere:
      return ground_point(region.position, world, object_classes, active_layers);
   case region_shape::cylinder:
      return ground_region_cylinder(region.size, region.rotation, region.position,
                                    world, object_classes, active_layers);
   default:
      return std::nullopt;
   }
}

auto ground_sector(const sector& sector, const world& world,
                   const object_class_library& object_classes,
                   const active_layers active_layers) noexcept -> std::optional<float>
{
   float2 point_min{FLT_MAX, FLT_MAX};
   float2 point_max{-FLT_MAX, -FLT_MAX};

   for (const float2 point : sector.points) {
      point_min = min(point_min, point);
      point_max = max(point_max, point);
   }

   const float2 point_centre = (point_min + point_max) / 2.0f;

   const float3 ground_position = {point_centre.x, sector.base, point_centre.y};

   const std::optional<float3> result =
      ground_bbox(ground_position,
                  {{point_min.x, sector.base, point_min.y},
                   {point_max.x, sector.base + sector.height, point_max.y}},
                  world, object_classes, active_layers);

   return result ? std::optional{result->y} : std::nullopt;
}

auto ground_portal(const portal& portal, const world& world,
                   const object_class_library& object_classes,
                   const active_layers active_layers) noexcept -> std::optional<float3>
{
   const float3 size{portal.width / 2.0f, portal.height / 2.0f, 0.0f};

   return ground_bbox(portal.position,
                      portal.rotation * math::bounding_box{-size, size} + portal.position,
                      world, object_classes, active_layers);
}

auto ground_point(const float3 point, const world& world,
                  const object_class_library& object_classes,
                  const active_layers active_layers) noexcept -> std::optional<float3>
{
   float hit_distance = std::numeric_limits<float>::max();

   const float3 ray_origin = point;

   if (std::optional<raycast_result<object>> hit =
          raycast(ray_origin, {0.0f, -1.0f, 0.0f}, active_layers, world.objects,
                  object_classes);
       hit) {
      if (hit->distance < hit_distance) {
         hit_distance = hit->distance;
      }
   }

   if (std::optional<raycast_block_result> hit =
          raycast(ray_origin, {0.0f, -1.0f, 0.0f}, active_layers, world.blocks);
       hit) {
      if (hit->distance < hit_distance) {
         hit_distance = hit->distance;
      }
   }

   if (auto hit = raycast(ray_origin, {0.0f, -1.0f, 0.0f}, world.terrain); hit) {
      if (*hit < hit_distance) hit_distance = *hit;
   }

   if (hit_distance != std::numeric_limits<float>::max()) {
      const float3 new_position = {point.x, point.y - hit_distance, point.z};

      if (new_position != point) return new_position;
   }

   return std::nullopt;
}

}