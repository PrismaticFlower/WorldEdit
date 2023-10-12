#include "terrain_cut.hpp"
#include "../object_class.hpp"
#include "assets/msh/flat_model.hpp"
#include "math/intersectors.hpp"
#include "math/quaternion_funcs.hpp"

namespace we::world {

namespace {

auto get_plane(float3 v0, float3 v1, float3 v2) noexcept -> float4
{
   const float3 edge0 = v1 - v0;
   const float3 edge1 = v2 - v0;
   const float3 normal = normalize(cross(edge0, edge1));

   return float4{normal, -dot(normal, v0)};
}

}

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

auto gather_terrain_cuts(std::span<const object> objects,
                         const object_class_library& object_classes)
   -> std::vector<terrain_cut>
{
   using namespace assets;

   std::vector<terrain_cut> terrain_cuts;
   terrain_cuts.reserve(128);

   for (auto& object : objects) {
      const msh::flat_model& model = *object_classes[object.class_name].model;

      if (model.terrain_cuts.empty()) continue;

      for (auto& cut : model.terrain_cuts) {
         math::bounding_box bbox = {.min =
                                       float3{std::numeric_limits<float>::max(),
                                              std::numeric_limits<float>::max(),
                                              std::numeric_limits<float>::max()},
                                    .max =
                                       float3{std::numeric_limits<float>::lowest(),
                                              std::numeric_limits<float>::lowest(),
                                              std::numeric_limits<float>::lowest()}};

         for (const auto& pos : cut.positions) {
            bbox = math::integrate(bbox, object.rotation * pos + object.position);
         }

         std::vector<float4> planes;
         planes.reserve(cut.triangles.size());

         for (const auto& tri : cut.triangles) {
            planes.push_back(
               get_plane(object.rotation * cut.positions[tri[0]] + object.position,
                         object.rotation * cut.positions[tri[1]] + object.position,
                         object.rotation * cut.positions[tri[2]] + object.position));
         }

         terrain_cuts.emplace_back(bbox, std::move(planes));
      }
   }

   return terrain_cuts;
}

}