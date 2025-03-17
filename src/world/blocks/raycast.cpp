#include "raycast.hpp"

#include "math/intersectors.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const blocks_boxes& boxes) noexcept -> std::optional<raycast_block_result>
{
   float closest = FLT_MAX;
   uint32 closest_index = UINT32_MAX;

   for (uint32 box_index = 0; box_index < boxes.size(); ++box_index) {
      const block_description_box& box = boxes.description[box_index];

      const quaternion cube_from_world = conjugate(box.rotation);
      const float3 ray_originCS = cube_from_world * ray_originWS - box.position;
      const float3 ray_directionCS = cube_from_world * ray_directionWS;

      if (float hit = 0.0f; intersect_aabb(ray_originCS, 1.0f / ray_directionCS,
                                           {-box.size, box.size}, closest, hit)) {
         closest = hit;
         closest_index = box_index;
      }
   }

   return closest_index != UINT32_MAX
             ? std::optional{raycast_block_result{.distance = closest,
                                                  .index = closest_index}}
             : std::nullopt;
}

}
