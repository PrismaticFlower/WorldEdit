#include "raycast.hpp"
#include "mesh_geometry.hpp"

#include "math/iq_intersectors.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

namespace {

struct raycast_block_result_local {
   float distance = 0.0f;
   uint32 index = 0;
   uint32 surface_index = 0;
};

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers, const blocks_boxes& boxes,
             const float max_distance,
             function_ptr<bool(const block_id id) noexcept> filter) noexcept
   -> std::optional<raycast_block_result_local>
{
   float closest = max_distance;
   uint32 closest_index = UINT32_MAX;

   for (uint32 box_index = 0; box_index < boxes.size(); ++box_index) {
      if (not active_layers[boxes.layer[box_index]]) continue;
      if (boxes.hidden[box_index]) continue;

      const block_description_box& box = boxes.description[box_index];

      const quaternion local_from_world = conjugate(box.rotation);

      const float3 positionLS = local_from_world * -box.position;

      const float3 ray_originLS = local_from_world * ray_originWS + positionLS;
      const float3 ray_directionLS = normalize(local_from_world * ray_directionWS);

      if (const float hit = boxIntersection(ray_originLS, ray_directionLS, box.size);
          hit >= 0.0f and hit < closest) {
         if (filter and not filter(boxes.ids[box_index])) continue;

         closest = hit;
         closest_index = box_index;
      }
   }

   if (closest_index != UINT32_MAX) {
      const block_description_box& box = boxes.description[closest_index];

      const float4x4 scale = {
         {box.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, box.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, box.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(box.rotation);

      float4x4 world_from_object = rotation * scale;
      world_from_object[3] = {box.position, 1.0f};

      uint32 surface_index = UINT32_MAX;
      float closest_surface = FLT_MAX;

      for (const std::array<uint16, 3>& tri : block_cube_triangles) {
         const float3 pos0WS = world_from_object * block_cube_vertices[tri[0]].position;
         const float3 pos1WS = world_from_object * block_cube_vertices[tri[1]].position;
         const float3 pos2WS = world_from_object * block_cube_vertices[tri[2]].position;

         const float3 hit =
            triIntersect(ray_originWS, ray_directionWS, pos0WS, pos1WS, pos2WS);

         if (hit.x >= 0.0f and hit.x < closest_surface) {
            surface_index = block_cube_vertices[tri[0]].surface_index;
            closest_surface = hit.x;
         }
      }

      if (surface_index != UINT32_MAX) {
         return raycast_block_result_local{.distance = closest,
                                           .index = closest_index,
                                           .surface_index = surface_index};
      }
   }

   return std::nullopt;
}

}

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers, const blocks& blocks,
             function_ptr<bool(const block_id id) noexcept> filter) noexcept
   -> std::optional<raycast_block_result>
{
   float closest = FLT_MAX;
   block_id closest_id = {};
   uint32 closest_index = UINT32_MAX;
   uint32 closest_surface_index = UINT32_MAX;

   if (std::optional<raycast_block_result_local> hit =
          raycast(ray_originWS, ray_directionWS, active_layers, blocks.boxes,
                  closest, filter);
       hit) {
      closest = hit->distance;
      closest_id = blocks.boxes.ids[hit->index];
      closest_index = hit->index;
      closest_surface_index = hit->surface_index;
   }

   if (closest_index != UINT32_MAX) {
      return raycast_block_result{
         .distance = closest,
         .id = closest_id,
         .index = closest_index,
         .surface_index = closest_surface_index,
      };
   }

   return std::nullopt;
}

}
