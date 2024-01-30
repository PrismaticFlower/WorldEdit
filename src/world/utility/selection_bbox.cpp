#include "selection_bbox.hpp"
#include "../object_class.hpp"
#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "world_utilities.hpp"

namespace we::world {

auto selection_bbox_for_camera(const world& world,
                               const std::span<const selected_entity> selection,
                               const object_class_library& object_classes,
                               const float path_node_size) -> math::bounding_box
{
   if (selection.empty()) return {};

   math::bounding_box selection_bbox{.min = float3{FLT_MAX, FLT_MAX, FLT_MAX},
                                     .max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX}};

   for (const auto& selected : selection) {
      if (std::holds_alternative<object_id>(selected)) {
         const object* object =
            find_entity(world.objects, std::get<object_id>(selected));

         if (object) {
            math::bounding_box bbox =
               object_classes[object->class_name].model->bounding_box;

            bbox = object->rotation * bbox + object->position;

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<path_id_node_pair>(selected)) {
         const auto [id, node_index] = std::get<path_id_node_pair>(selected);

         const path* path = find_entity(world.paths, id);

         if (path) {
            const path::node& node = path->nodes[node_index];

            const math::bounding_box bbox{.min = node.position - path_node_size,
                                          .max = node.position + path_node_size};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<light_id>(selected)) {
         const light* light = find_entity(world.lights, std::get<light_id>(selected));

         if (light) {
            switch (light->light_type) {
            case light_type::directional: {
               const math::bounding_box bbox{.min = light->position - 5.0f,
                                             .max = light->position + 5.0f};

               selection_bbox = math::combine(bbox, selection_bbox);

            } break;
            case light_type::point: {
               const math::bounding_box bbox{.min = light->position - light->range,
                                             .max = light->position + light->range};

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case light_type::spot: {
               const float half_range = light->range / 2.0f;
               const float outer_cone_radius =
                  half_range * std::tan(light->outer_cone_angle);

               math::bounding_box bbox{.min = {-outer_cone_radius, 0.0f, -outer_cone_radius},
                                       .max = {outer_cone_radius, light->range,
                                               outer_cone_radius}};

               bbox = light->rotation * bbox + light->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case light_type::directional_region_box: {
               math::bounding_box bbox{.min = {-light->region_size},
                                       .max = {light->region_size}};

               bbox = light->region_rotation * bbox + light->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case light_type::directional_region_sphere: {
               const float sphere_radius = length(light->region_size);

               const math::bounding_box bbox{.min = light->position - sphere_radius,
                                             .max = light->position + sphere_radius};

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case light_type::directional_region_cylinder: {
               const float cylinder_length =
                  length(float2{light->region_size.x, light->region_size.z});

               math::bounding_box bbox{.min = {-cylinder_length,
                                               -light->region_size.y, -cylinder_length},
                                       .max = {cylinder_length,
                                               light->region_size.y, cylinder_length}};

               bbox = light->region_rotation * bbox + light->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            }
         }
      }
      else if (std::holds_alternative<region_id>(selected)) {
         const region* region =
            find_entity(world.regions, std::get<region_id>(selected));

         if (region) {
            switch (region->shape) {
            case region_shape::box: {
               math::bounding_box bbox{.min = {-region->size}, .max = {region->size}};

               bbox = region->rotation * bbox + region->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case region_shape::sphere: {
               const float sphere_radius = length(region->size);

               const math::bounding_box bbox{.min = region->position - sphere_radius,
                                             .max = region->position + sphere_radius};

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case region_shape::cylinder: {
               const float cylinder_length =
                  length(float2{region->size.x, region->size.z});

               math::bounding_box bbox{.min = {-cylinder_length, -region->size.y,
                                               -cylinder_length},
                                       .max = {cylinder_length, region->size.y,
                                               cylinder_length}};

               bbox = region->rotation * bbox + region->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            }
         }
      }
      else if (std::holds_alternative<sector_id>(selected)) {
         const sector* sector =
            find_entity(world.sectors, std::get<sector_id>(selected));

         if (sector) {
            math::bounding_box bbox{.min = {FLT_MAX, sector->base, FLT_MAX},
                                    .max = {-FLT_MAX, sector->base + sector->height,
                                            -FLT_MAX}};

            for (const auto& point : sector->points) {
               bbox.min.x = std::min(point.x, bbox.min.x);
               bbox.min.z = std::min(point.y, bbox.min.z);

               bbox.max.x = std::max(point.x, bbox.max.x);
               bbox.max.z = std::max(point.y, bbox.max.z);
            }

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<portal_id>(selected)) {
         const portal* portal =
            find_entity(world.portals, std::get<portal_id>(selected));

         if (portal) {
            const float half_width = portal->width * 0.5f;
            const float half_height = portal->height * 0.5f;

            math::bounding_box bbox{.min = {-half_width, -half_height, 0.0f},
                                    .max = {half_width, half_height, 0.0f}};

            bbox = portal->rotation * bbox + portal->position;

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<hintnode_id>(selected)) {
         const hintnode* hintnode =
            find_entity(world.hintnodes, std::get<hintnode_id>(selected));

         if (hintnode) {
            const math::bounding_box bbox{.min = hintnode->position - 3.0f,
                                          .max = hintnode->position + 3.0f};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<barrier_id>(selected)) {
         const barrier* barrier =
            find_entity(world.barriers, std::get<barrier_id>(selected));

         if (barrier) {
            const float radius = std::max(barrier->size.x, barrier->size.y);

            const math::bounding_box bbox{.min = barrier->position - radius,
                                          .max = barrier->position + radius};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<planning_hub_id>(selected)) {
         const planning_hub* hub =
            find_entity(world.planning_hubs, std::get<planning_hub_id>(selected));

         if (hub) {
            const math::bounding_box bbox{.min = hub->position - hub->radius,
                                          .max = hub->position + hub->radius};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<planning_connection_id>(selected)) {
         const planning_connection* connection =
            find_entity(world.planning_connections,
                        std::get<planning_connection_id>(selected));

         if (connection) {
            const planning_hub& start =
               world.planning_hubs[connection->start_hub_index];
            const planning_hub& end = world.planning_hubs[connection->end_hub_index];

            const math::bounding_box start_bbox{
               .min = float3{-start.radius, 0.0f, -start.radius} + start.position,
               .max = float3{start.radius, 0.0f, start.radius} + start.position};
            const math::bounding_box end_bbox{
               .min = float3{-end.radius, 0.0f, -end.radius} + end.position,
               .max = float3{end.radius, 0.0f, end.radius} + end.position};

            const math::bounding_box bbox = math::combine(start_bbox, end_bbox);

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<boundary_id>(selected)) {
         const boundary* boundary =
            find_entity(world.boundaries, std::get<boundary_id>(selected));

         if (boundary) {
            const float2 boundary_min = boundary->position - boundary->size;
            const float2 boundary_max = boundary->position + boundary->size;

            const math::bounding_box bbox{.min = {boundary_min.x, 0.0f,
                                                  boundary_min.y},
                                          .max = {boundary_max.x, 0.0f,
                                                  boundary_max.y}};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<measurement_id>(selected)) {
         const measurement* measurement =
            find_entity(world.measurements, std::get<measurement_id>(selected));

         if (measurement) {
            selection_bbox = math::integrate(selection_bbox, measurement->start);
            selection_bbox = math::integrate(selection_bbox, measurement->end);
         }
      }
   }

   return selection_bbox;
}

auto selection_bbox_for_move(const world& world,
                             const std::span<const selected_entity> selection,
                             const object_class_library& object_classes) -> math::bounding_box
{
   math::bounding_box selection_bbox{.min = float3{FLT_MAX, FLT_MAX, FLT_MAX},
                                     .max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX}};

   for (const auto& selected : selection) {
      if (std::holds_alternative<object_id>(selected)) {
         const object* object =
            find_entity(world.objects, std::get<object_id>(selected));

         if (object) {
            math::bounding_box bbox =
               object_classes[object->class_name].model->bounding_box;

            bbox = object->rotation * bbox + object->position;

            selection_bbox = math::combine(selection_bbox, bbox);
         }
      }
      else if (std::holds_alternative<path_id_node_pair>(selected)) {
         const auto [id, node_index] = std::get<path_id_node_pair>(selected);

         const path* path = find_entity(world.paths, id);

         if (path) {
            selection_bbox =
               math::integrate(selection_bbox, path->nodes[node_index].position);
         }
      }
      else if (std::holds_alternative<light_id>(selected)) {
         const light* light = find_entity(world.lights, std::get<light_id>(selected));

         if (light) {
            switch (light->light_type) {
            case light_type::directional: {
               const math::bounding_box bbox{.min = light->position - 5.0f,
                                             .max = light->position + 5.0f};

               selection_bbox = math::combine(bbox, selection_bbox);

            } break;
            case light_type::point: {
               const math::bounding_box bbox{.min = light->position - light->range,
                                             .max = light->position + light->range};

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case light_type::spot: {
               const float half_range = light->range / 2.0f;
               const float outer_cone_radius =
                  half_range * std::tan(light->outer_cone_angle);

               math::bounding_box bbox{.min = {-outer_cone_radius, 0.0f, -outer_cone_radius},
                                       .max = {outer_cone_radius, light->range,
                                               outer_cone_radius}};

               bbox = light->rotation * bbox + light->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case light_type::directional_region_box: {
               math::bounding_box bbox{.min = {-light->region_size},
                                       .max = {light->region_size}};

               bbox = light->region_rotation * bbox + light->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case light_type::directional_region_sphere: {
               const float sphere_radius = length(light->region_size);

               const math::bounding_box bbox{.min = light->position - sphere_radius,
                                             .max = light->position + sphere_radius};

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case light_type::directional_region_cylinder: {
               const float cylinder_length =
                  length(float2{light->region_size.x, light->region_size.z});

               math::bounding_box bbox{.min = {-cylinder_length,
                                               -light->region_size.y, -cylinder_length},
                                       .max = {cylinder_length,
                                               light->region_size.y, cylinder_length}};

               bbox = light->region_rotation * bbox + light->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            }
         }
      }
      else if (std::holds_alternative<region_id>(selected)) {
         const region* region =
            find_entity(world.regions, std::get<region_id>(selected));

         if (region) {
            switch (region->shape) {
            case region_shape::box: {
               math::bounding_box bbox{.min = {-region->size}, .max = {region->size}};

               bbox = region->rotation * bbox + region->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case region_shape::sphere: {
               const float sphere_radius = length(region->size);

               const math::bounding_box bbox{.min = region->position - sphere_radius,
                                             .max = region->position + sphere_radius};

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            case region_shape::cylinder: {
               const float cylinder_length =
                  length(float2{region->size.x, region->size.z});

               math::bounding_box bbox{.min = {-cylinder_length, -region->size.y,
                                               -cylinder_length},
                                       .max = {cylinder_length, region->size.y,
                                               cylinder_length}};

               bbox = region->rotation * bbox + region->position;

               selection_bbox = math::combine(bbox, selection_bbox);
            } break;
            }
         }
      }
      else if (std::holds_alternative<sector_id>(selected)) {
         const sector* sector =
            find_entity(world.sectors, std::get<sector_id>(selected));

         if (sector) {
            for (auto& point : sector->points) {
               selection_bbox.min.x = std::min(selection_bbox.min.x, point.x);
               selection_bbox.min.z = std::min(selection_bbox.min.z, point.y);

               selection_bbox.max.x = std::max(selection_bbox.max.x, point.x);
               selection_bbox.max.z = std::max(selection_bbox.max.z, point.y);
            }

            selection_bbox.min.y = std::min(selection_bbox.min.y, sector->base);
            selection_bbox.max.y =
               std::max(selection_bbox.max.y, sector->base + sector->height);
         }
      }
      else if (std::holds_alternative<portal_id>(selected)) {
         const portal* portal =
            find_entity(world.portals, std::get<portal_id>(selected));

         if (portal) {
            const float half_width = portal->width * 0.5f;
            const float half_height = portal->height * 0.5f;

            math::bounding_box bbox{.min = {-half_width, -half_height, 0.0f},
                                    .max = {half_width, half_height, 0.0f}};

            bbox = portal->rotation * bbox + portal->position;

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<hintnode_id>(selected)) {
         const hintnode* hintnode =
            find_entity(world.hintnodes, std::get<hintnode_id>(selected));

         if (hintnode) {
            selection_bbox = math::integrate(selection_bbox, hintnode->position);
         }
      }
      else if (std::holds_alternative<barrier_id>(selected)) {
         const barrier* barrier =
            find_entity(world.barriers, std::get<barrier_id>(selected));

         if (barrier) {
            selection_bbox = math::integrate(selection_bbox, barrier->position);
         }
      }
      else if (std::holds_alternative<planning_hub_id>(selected)) {
         const planning_hub* planning_hub =
            find_entity(world.planning_hubs, std::get<planning_hub_id>(selected));

         if (planning_hub) {
            selection_bbox = math::integrate(selection_bbox, planning_hub->position);
         }
      }
      else if (std::holds_alternative<boundary_id>(selected)) {
         const boundary* boundary =
            find_entity(world.boundaries, std::get<boundary_id>(selected));

         if (boundary) {
            const float2 boundary_min = boundary->position - boundary->size;
            const float2 boundary_max = boundary->position + boundary->size;

            const math::bounding_box bbox{.min = {boundary_min.x, 0.0f,
                                                  boundary_min.y},
                                          .max = {boundary_max.x, 0.0f,
                                                  boundary_max.y}};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (std::holds_alternative<measurement_id>(selected)) {
         const measurement* measurement =
            find_entity(world.measurements, std::get<measurement_id>(selected));

         if (measurement) {
            selection_bbox = math::integrate(selection_bbox, measurement->start);
            selection_bbox = math::integrate(selection_bbox, measurement->end);
         }
      }
   }

   return selection_bbox;
}

}