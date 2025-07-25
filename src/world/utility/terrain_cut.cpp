#include "terrain_cut.hpp"

#include "../blocks/mesh_geometry.hpp"
#include "../object_class.hpp"

#include "assets/msh/flat_model.hpp"

#include "math/iq_intersectors.hpp"
#include "math/plane_funcs.hpp"
#include "math/quaternion_funcs.hpp"

namespace we::world {

bool point_inside_terrain_cut(const float3 point, const float3 ray_direction,
                              const active_layers active_layers, const world& world,
                              const object_class_library& object_classes) noexcept
{
   using namespace assets;

   for (const object& object : world.objects) {
      if (not active_layers[object.layer]) continue;
      if (object.hidden) continue;

      const msh::flat_model& model = *object_classes[object.class_handle].model;

      if (model.terrain_cuts.empty()) continue;

      const quaternion object_from_world = conjugate(object.rotation);

      const float3 pointOS = object_from_world * (point - object.position);
      const float3 ray_directionOS = object_from_world * ray_direction;

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

   for (const block_description_terrain_cut_box& box :
        world.blocks.terrain_cut_boxes.description) {
      const quaternion local_from_world = conjugate(box.rotation);
      const float3 pointLS = local_from_world * (point - box.position);

      if (length(max(abs(pointLS) - box.size, float3{0.0f, 0.0f, 0.0f})) == 0.0f) {
         return true;
      }
   }

   return false;
}

auto gather_terrain_cuts(const world& world, const object_class_library& object_classes)
   -> std::vector<terrain_cut>
{
   using namespace assets;

   std::vector<terrain_cut> terrain_cuts;
   terrain_cuts.reserve(128);

   for (const object& object : world.objects) {
      const msh::flat_model& model = *object_classes[object.class_handle].model;

      if (model.terrain_cuts.empty()) continue;

      float4x4 world_from_object_ti = to_matrix(normalize(object.rotation));
      world_from_object_ti[3] = {object.position, 1.0f};
      world_from_object_ti = transpose(inverse(world_from_object_ti));

      for (const msh::flat_model_terrain_cut& cut : model.terrain_cuts) {
         math::bounding_box bbox = {.min =
                                       float3{std::numeric_limits<float>::max(),
                                              std::numeric_limits<float>::max(),
                                              std::numeric_limits<float>::max()},
                                    .max =
                                       float3{std::numeric_limits<float>::lowest(),
                                              std::numeric_limits<float>::lowest(),
                                              std::numeric_limits<float>::lowest()}};

         for (const float3& pos : cut.positions) {
            bbox = math::integrate(bbox, object.rotation * pos + object.position);
         }

         std::vector<float4> planes = cut.planes;

         for (float4& plane : planes) {
            const float4 new_planeWS = world_from_object_ti * plane;
            const float3 normalWS =
               normalize(float3{new_planeWS.x, new_planeWS.y, new_planeWS.z});

            plane = float4{normalWS, new_planeWS.w};
         }

         terrain_cuts.emplace_back(bbox, std::move(planes));
      }
   }

   for (std::size_t i = 0; i < world.blocks.terrain_cut_boxes.size(); ++i) {
      const block_description_terrain_cut_box& box =
         world.blocks.terrain_cut_boxes.description[i];

      const math::bounding_box bbox =
         {.min = float3{world.blocks.terrain_cut_boxes.bbox.min_x[i],
                        world.blocks.terrain_cut_boxes.bbox.min_y[i],
                        world.blocks.terrain_cut_boxes.bbox.min_z[i]},
          .max = float3{world.blocks.terrain_cut_boxes.bbox.max_x[i],
                        world.blocks.terrain_cut_boxes.bbox.max_y[i],
                        world.blocks.terrain_cut_boxes.bbox.max_z[i]}};

      float4x4 world_from_local_ti = to_matrix(box.rotation);
      world_from_local_ti[3] = {box.position, 1.0f};
      world_from_local_ti = transpose(inverse(world_from_local_ti));

      std::vector<float4> planes = {
         {1.0f, 0.0f, 0.0f, -box.size.x}, {-1.0f, 0.0f, 0.0f, -box.size.x},
         {0.0f, 1.0f, 0.0f, -box.size.y}, {0.0f, -1.0f, 0.0f, -box.size.y},
         {0.0f, 0.0f, 1.0f, -box.size.z}, {0.0f, 0.0f, -1.0f, -box.size.z},
      };

      for (float4& plane : planes) {
         const float4 plane_unormWS = world_from_local_ti * plane;
         const float3 normalWS =
            normalize(float3{plane_unormWS.x, plane_unormWS.y, plane_unormWS.z});

         plane = float4{normalWS, plane_unormWS.w};
      }

      terrain_cuts.emplace_back(bbox, std::move(planes));
   }

   return terrain_cuts;
}

}