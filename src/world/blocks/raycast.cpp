#include "raycast.hpp"

#include "math/iq_intersectors.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const blocks_boxes& boxes) noexcept -> std::optional<raycast_block_result>
{
   float closest = FLT_MAX;
   uint32 closest_index = UINT32_MAX;

   for (uint32 box_index = 0; box_index < boxes.size(); ++box_index) {
      if (boxes.hidden[box_index]) continue;

      const block_description_box& box = boxes.description[box_index];

      const quaternion local_from_world = conjugate(box.rotation);

      const float3 positionLS = local_from_world * -box.position;

      const float3 ray_originLS = local_from_world * ray_originWS + positionLS;
      const float3 ray_directionLS = normalize(local_from_world * ray_directionWS);

      if (const float hit = boxIntersection(ray_originLS, ray_directionLS, box.size);
          hit >= 0.0f and hit < closest) {
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
