#include "grounding.hpp"
#include "bounding_box.hpp"
#include "raycast.hpp"

#include "../utility/raycast.hpp"
#include "../utility/raycast_terrain.hpp"

#include "math/vector_funcs.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

auto ground_block(const float3& position, const math::bounding_box& bbox,
                  const block_id id, const world& world,
                  const object_class_library& object_classes,
                  const active_layers active_layers) noexcept -> std::optional<float3>
{
   float hit_distance = FLT_MAX;

   const float3 ray_origin = {position.x, bbox.min.y, position.z};

   const auto raycast_filter = [id](const block_id other_id) noexcept {
      return other_id != id;
   };

   if (std::optional<raycast_block_result> hit =
          raycast(ray_origin, {0.0f, -1.0f, 0.0f}, active_layers, world.blocks,
                  raycast_filter);
       hit) {
      if (hit->distance < hit_distance) {
         hit_distance = hit->distance;
      }
   }

   if (std::optional<raycast_result<object>> hit =
          raycast(ray_origin, {0.0f, -1.0f, 0.0f}, active_layers, world.objects,
                  object_classes);
       hit) {
      if (hit->distance < hit_distance) {
         hit_distance = hit->distance;
      }
   }

   if (auto hit = raycast(ray_origin, {0.0f, -1.0f, 0.0f}, world.terrain); hit) {
      if (*hit < hit_distance) hit_distance = *hit;
   }

   // Try "digging" the BBOX out of the ground.
   if (hit_distance == FLT_MAX) {
      if (std::optional<raycast_block_result> hit =
             raycast(ray_origin, {0.0f, 1.0f, 0.0f}, active_layers,
                     world.blocks, raycast_filter);
          hit) {
         if (hit->distance < hit_distance) {
            hit_distance = hit->distance;
         }
      }

      if (std::optional<raycast_result<object>> hit =
             raycast(ray_origin, {0.0f, 1.0f, 0.0f}, active_layers,
                     world.objects, object_classes);
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
         hit_distance = FLT_MAX;
      }
   }

   if (hit_distance != FLT_MAX) {
      const float3 new_position = {position.x, position.y - hit_distance,
                                   position.z};

      if (new_position != position) return new_position;
   }

   return std::nullopt;
}

}

auto ground_block(const block_id id, const uint32 block_index,
                  const world& world, const object_class_library& object_classes,
                  const active_layers active_layers) noexcept -> std::optional<float3>
{
   const math::bounding_box bbox =
      get_bounding_box(world.blocks, id.type(), block_index);

   switch (id.type()) {
   case block_type::box: {
      const block_description_box& box = world.blocks.boxes.description[block_index];

      return ground_block(box.position, bbox, id, world, object_classes, active_layers);
   } break;
   case block_type::ramp: {
      const block_description_ramp& ramp = world.blocks.ramps.description[block_index];

      return ground_block(ramp.position, bbox, id, world, object_classes, active_layers);
   } break;
   case block_type::quad: {
      return ground_block((bbox.min + bbox.max) / 2.0f, bbox, id, world,
                          object_classes, active_layers);
   } break;
   case block_type::cylinder: {
      const block_description_cylinder& cylinder =
         world.blocks.cylinders.description[block_index];

      return ground_block(cylinder.position, bbox, id, world, object_classes,
                          active_layers);
   } break;
   case block_type::stairway: {
      const block_description_stairway& stairway =
         world.blocks.stairways.description[block_index];

      return ground_block(stairway.position, bbox, id, world, object_classes,
                          active_layers);
   } break;
   case block_type::cone: {
      const block_description_cone& cone = world.blocks.cones.description[block_index];

      return ground_block(cone.position, bbox, id, world, object_classes, active_layers);
   } break;
   case block_type::hemisphere: {
      const block_description_hemisphere& hemisphere =
         world.blocks.hemispheres.description[block_index];

      return ground_block(hemisphere.position, bbox, id, world, object_classes,
                          active_layers);
   } break;
   case block_type::pyramid: {
      const block_description_pyramid& pyramid =
         world.blocks.pyramids.description[block_index];

      return ground_block(pyramid.position, bbox, id, world, object_classes,
                          active_layers);
   } break;
   }

   std::unreachable();
}

}