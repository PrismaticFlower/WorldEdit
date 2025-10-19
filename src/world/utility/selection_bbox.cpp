#include "selection_bbox.hpp"
#include "world_utilities.hpp"

#include "../blocks/utility/bounding_box.hpp"
#include "../blocks/utility/find.hpp"
#include "../object_class.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

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
      if (selected.is<object_id>()) {
         const object* object =
            find_entity(world.objects, selected.get<object_id>());

         if (object) {
            math::bounding_box bbox =
               object_classes[object->class_handle].model->bounding_box;

            bbox = object->rotation * bbox + object->position;

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (selected.is<path_id_node_mask>()) {
         const auto& [id, node_mask] = selected.get<path_id_node_mask>();

         const path* path = find_entity(world.paths, id);

         if (path) {
            const std::size_t node_count =
               std::min(path->nodes.size(), max_path_nodes);

            for (uint32 node_index = 0; node_index < node_count; ++node_index) {
               if (not node_mask[node_index]) continue;

               const path::node& node = path->nodes[node_index];

               const math::bounding_box bbox{.min = node.position - path_node_size,
                                             .max = node.position + path_node_size};

               selection_bbox = math::combine(bbox, selection_bbox);
            }
         }
      }
      else if (selected.is<light_id>()) {
         const light* light = find_entity(world.lights, selected.get<light_id>());

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
               const float outer_cone_radius =
                  light->range * std::tan(light->outer_cone_angle * 0.5f);
               const float3 light_directionWS =
                  normalize(light->rotation * float3{0.0f, 0.0f, 1.0f});
               const float3 cone_baseWS =
                  light->position + light_directionWS * light->range;
               const float3 e = outer_cone_radius *
                                sqrt(1.0f - light_directionWS * light_directionWS);

               const math::bounding_box bbox{.min = min(cone_baseWS - e, light->position),
                                             .max = max(cone_baseWS + e, light->position)};

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
      else if (selected.is<region_id>()) {
         const region* region =
            find_entity(world.regions, selected.get<region_id>());

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
      else if (selected.is<sector_id>()) {
         const sector* sector =
            find_entity(world.sectors, selected.get<sector_id>());

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
      else if (selected.is<portal_id>()) {
         const portal* portal =
            find_entity(world.portals, selected.get<portal_id>());

         if (portal) {
            const float half_width = portal->width * 0.5f;
            const float half_height = portal->height * 0.5f;

            math::bounding_box bbox{.min = {-half_width, -half_height, 0.0f},
                                    .max = {half_width, half_height, 0.0f}};

            bbox = portal->rotation * bbox + portal->position;

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (selected.is<hintnode_id>()) {
         const hintnode* hintnode =
            find_entity(world.hintnodes, selected.get<hintnode_id>());

         if (hintnode) {
            const math::bounding_box bbox{.min = hintnode->position - 3.0f,
                                          .max = hintnode->position + 3.0f};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (selected.is<barrier_id>()) {
         const barrier* barrier =
            find_entity(world.barriers, selected.get<barrier_id>());

         if (barrier) {
            const float radius = std::max(barrier->size.x, barrier->size.y);

            const math::bounding_box bbox{.min = barrier->position - radius,
                                          .max = barrier->position + radius};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (selected.is<planning_hub_id>()) {
         const planning_hub* hub =
            find_entity(world.planning_hubs, selected.get<planning_hub_id>());

         if (hub) {
            const math::bounding_box bbox{.min = hub->position - hub->radius,
                                          .max = hub->position + hub->radius};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (selected.is<planning_connection_id>()) {
         const planning_connection* connection =
            find_entity(world.planning_connections,
                        selected.get<planning_connection_id>());

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
      else if (selected.is<boundary_id>()) {
         const boundary* boundary =
            find_entity(world.boundaries, selected.get<boundary_id>());

         if (boundary) {
            const math::bounding_box bbox{.min = boundary->position -
                                                 float3{boundary->size.x, 0.0f,
                                                        boundary->size.y},
                                          .max = boundary->position +
                                                 float3{boundary->size.x, 0.0f,
                                                        boundary->size.y}};

            selection_bbox = math::combine(bbox, selection_bbox);
         }
      }
      else if (selected.is<measurement_id>()) {
         const measurement* measurement =
            find_entity(world.measurements, selected.get<measurement_id>());

         if (measurement) {
            selection_bbox = math::integrate(selection_bbox, measurement->start);
            selection_bbox = math::integrate(selection_bbox, measurement->end);
         }
      }
      else if (selected.is<block_id>()) {
         const block_id id = selected.get<block_id>();
         const std::optional<uint32> block_index = find_block(world.blocks, id);

         if (block_index) {
            selection_bbox =
               math::combine(selection_bbox,
                             get_bounding_box(world.blocks, id.type(), *block_index));
         }
      }
   }

   return selection_bbox;
}

auto selection_metrics_for_move(const world& world,
                                const std::span<const selected_entity> selection,
                                const object_class_library& object_classes) -> selection_metrics
{
   math::bounding_box selection_bbox{.min = float3{FLT_MAX, FLT_MAX, FLT_MAX},
                                     .max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX}};
   float3 centreWS = {};
   float point_count = 0.0f;

   for (const auto& selected : selection) {
      if (selected.is<object_id>()) {
         const object* object =
            find_entity(world.objects, selected.get<object_id>());

         if (object) {
            math::bounding_box bbox =
               object_classes[object->class_handle].model->bounding_box;

            bbox = object->rotation * bbox + object->position;

            selection_bbox = math::combine(selection_bbox, bbox);

            centreWS += object->position;
            point_count += 1.0f;
         }
      }
      else if (selected.is<path_id_node_mask>()) {
         const auto& [id, node_mask] = selected.get<path_id_node_mask>();

         const path* path = find_entity(world.paths, id);

         if (path) {
            const std::size_t node_count =
               std::min(path->nodes.size(), max_path_nodes);

            for (uint32 node_index = 0; node_index < node_count; ++node_index) {
               if (not node_mask[node_index]) continue;

               selection_bbox =
                  math::integrate(selection_bbox, path->nodes[node_index].position);

               centreWS += path->nodes[node_index].position;
               point_count += 1.0f;
            }
         }
      }
      else if (selected.is<light_id>()) {
         const light* light = find_entity(world.lights, selected.get<light_id>());

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
               const float outer_cone_radius =
                  light->range * std::tan(light->outer_cone_angle * 0.5f);
               const float3 light_directionWS =
                  normalize(light->rotation * float3{0.0f, 0.0f, 1.0f});
               const float3 cone_baseWS =
                  light->position + light_directionWS * light->range;
               const float3 e = outer_cone_radius *
                                sqrt(1.0f - light_directionWS * light_directionWS);

               const math::bounding_box bbox{.min = min(cone_baseWS - e, light->position),
                                             .max = max(cone_baseWS + e, light->position)};

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

            centreWS += light->position;
            point_count += 1.0f;
         }
      }
      else if (selected.is<region_id>()) {
         const region* region =
            find_entity(world.regions, selected.get<region_id>());

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

            centreWS += region->position;
            point_count += 1.0f;
         }
      }
      else if (selected.is<sector_id>()) {
         const sector* sector =
            find_entity(world.sectors, selected.get<sector_id>());

         if (sector) {
            for (auto& point : sector->points) {
               selection_bbox.min.x = std::min(selection_bbox.min.x, point.x);
               selection_bbox.min.z = std::min(selection_bbox.min.z, point.y);

               selection_bbox.max.x = std::max(selection_bbox.max.x, point.x);
               selection_bbox.max.z = std::max(selection_bbox.max.z, point.y);

               centreWS += float3{point.x, sector->base, point.y};
               point_count += 1.0f;
            }

            selection_bbox.min.y = std::min(selection_bbox.min.y, sector->base);
            selection_bbox.max.y =
               std::max(selection_bbox.max.y, sector->base + sector->height);
         }
      }
      else if (selected.is<portal_id>()) {
         const portal* portal =
            find_entity(world.portals, selected.get<portal_id>());

         if (portal) {
            const float half_width = portal->width * 0.5f;
            const float half_height = portal->height * 0.5f;

            math::bounding_box bbox{.min = {-half_width, -half_height, 0.0f},
                                    .max = {half_width, half_height, 0.0f}};

            bbox = portal->rotation * bbox + portal->position;

            selection_bbox = math::combine(bbox, selection_bbox);

            centreWS += portal->position;
            point_count += 1.0f;
         }
      }
      else if (selected.is<hintnode_id>()) {
         const hintnode* hintnode =
            find_entity(world.hintnodes, selected.get<hintnode_id>());

         if (hintnode) {
            selection_bbox = math::integrate(selection_bbox, hintnode->position);

            centreWS += hintnode->position;
            point_count += 1.0f;
         }
      }
      else if (selected.is<barrier_id>()) {
         const barrier* barrier =
            find_entity(world.barriers, selected.get<barrier_id>());

         if (barrier) {
            selection_bbox = math::integrate(selection_bbox, barrier->position);

            centreWS += barrier->position;
            point_count += 1.0f;
         }
      }
      else if (selected.is<planning_hub_id>()) {
         const planning_hub* planning_hub =
            find_entity(world.planning_hubs, selected.get<planning_hub_id>());

         if (planning_hub) {
            selection_bbox = math::integrate(selection_bbox, planning_hub->position);

            centreWS += planning_hub->position;
            point_count += 1.0f;
         }
      }
      else if (selected.is<boundary_id>()) {
         const boundary* boundary =
            find_entity(world.boundaries, selected.get<boundary_id>());

         if (boundary) {
            const math::bounding_box bbox{.min = boundary->position -
                                                 float3{boundary->size.x, 0.0f,
                                                        boundary->size.y},
                                          .max = boundary->position +
                                                 float3{boundary->size.x, 0.0f,
                                                        boundary->size.y}};

            selection_bbox = math::combine(bbox, selection_bbox);

            centreWS += boundary->position;
            point_count += 1.0f;
         }
      }
      else if (selected.is<measurement_id>()) {
         const measurement* measurement =
            find_entity(world.measurements, selected.get<measurement_id>());

         if (measurement) {
            selection_bbox = math::integrate(selection_bbox, measurement->start);
            selection_bbox = math::integrate(selection_bbox, measurement->end);

            centreWS += measurement->start;
            centreWS += measurement->end;
            point_count += 2.0f;
         }
      }
      else if (selected.is<block_id>()) {
         const block_id id = selected.get<block_id>();
         const std::optional<uint32> block_index = find_block(world.blocks, id);

         if (block_index) {
            const math::bounding_box block_bbox =
               get_bounding_box(world.blocks, id.type(), *block_index);

            selection_bbox = math::combine(selection_bbox, block_bbox);

            centreWS += (block_bbox.min + block_bbox.max) * 0.5f;
            point_count += 1.0f;
         }
      }
   }

   return {selection_bbox, centreWS / std::max(point_count, 1.0f)};
}

}