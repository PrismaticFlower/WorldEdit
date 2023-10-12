#include "terrain_cut.hpp"
#include "../object_class.hpp"
#include "assets/msh/flat_model.hpp"
#include "math/intersectors.hpp"
#include "math/quaternion_funcs.hpp"

namespace we::world {

bool point_inside_terrain_cut(const float3 point, const float3 ray_direction,
                              const active_layers active_layers,
                              std::span<const object> objects,
                              const object_class_library& object_classes) noexcept
{
   using namespace assets;

   for (auto& object : objects) {
      if (not active_layers[object.layer]) continue;
      if (object.hidden) continue;

      const msh::flat_model& model = *object_classes[object.class_name].model;

      if (model.terrain_cuts.empty()) continue;

      const quaternion inverse_rotation = conjugate(object.rotation);

      const float3 pointOS = inverse_rotation * (point - object.position);
      const float3 ray_directionOS = inverse_rotation * ray_direction;

      const float3 box_centre =
         (model.terrain_cuts_bounding_box.min + model.terrain_cuts_bounding_box.max) * 0.5f;
      const float3 box_size =
         model.terrain_cuts_bounding_box.max - model.terrain_cuts_bounding_box.min;

      const float box_intersection =
         boxIntersection(pointOS - box_centre, ray_directionOS, box_size);

      if (box_intersection < 0.0f) continue;

      const uint32 intersections =
         model.terrain_cut_bvh.count_intersections(pointOS, ray_directionOS);

      if ((intersections % 2) == 1) {
         return true;
      }
   }

   return false;
}

}