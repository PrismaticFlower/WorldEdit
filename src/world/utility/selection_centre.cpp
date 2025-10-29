#include "selection_centre.hpp"
#include "world_utilities.hpp"

#include "../blocks/utility/bounding_box.hpp"
#include "../blocks/utility/find.hpp"
#include "../object_class.hpp"

#include "math/vector_funcs.hpp"

namespace we::world {

auto selection_centre_for_rotate_around(const world& world,
                                        const std::span<const selected_entity> selection)
   -> float3
{
   float3 selection_centre = {0.0f, 0.0f, 0.0f};
   float3 selection_axis_count = {0.0f, 0.0f, 0.0f};

   for (const auto& selected : selection) {
      if (selected.is<object_id>()) {
         const object* object =
            find_entity(world.objects, selected.get<object_id>());

         if (object) {
            selection_centre += object->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
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

               selection_centre += node.position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
            }
         }
      }
      else if (selected.is<light_id>()) {
         const light* light = find_entity(world.lights, selected.get<light_id>());

         if (light) {
            selection_centre += light->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<region_id>()) {
         const region* region =
            find_entity(world.regions, selected.get<region_id>());

         if (region) {
            selection_centre += region->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<sector_id>()) {
         const sector* sector =
            find_entity(world.sectors, selected.get<sector_id>());

         if (sector) {
            for (auto& point : sector->points) {
               selection_centre += {point.x, 0.0f, point.y};
               selection_axis_count += {1.0f, 0.0f, 1.0f};
            }
         }
      }
      else if (selected.is<portal_id>()) {
         const portal* portal =
            find_entity(world.portals, selected.get<portal_id>());

         if (portal) {
            selection_centre += portal->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<hintnode_id>()) {
         const hintnode* hintnode =
            find_entity(world.hintnodes, selected.get<hintnode_id>());

         if (hintnode) {
            selection_centre += hintnode->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<barrier_id>()) {
         const barrier* barrier =
            find_entity(world.barriers, selected.get<barrier_id>());

         if (barrier) {
            selection_centre += barrier->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<planning_hub_id>()) {
         const planning_hub* planning_hub =
            find_entity(world.planning_hubs, selected.get<planning_hub_id>());

         if (planning_hub) {
            selection_centre += planning_hub->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<boundary_id>()) {
         const boundary* boundary =
            find_entity(world.boundaries, selected.get<boundary_id>());

         if (boundary) {
            for (const float3& point : boundary->points) {
               selection_centre += point;
               selection_axis_count += 1.0f;
            }
         }
      }
      else if (selected.is<measurement_id>()) {
         const measurement* measurement =
            find_entity(world.measurements, selected.get<measurement_id>());

         if (measurement) {
            selection_centre += measurement->start;
            selection_centre += measurement->end;
            selection_axis_count += {2.0f, 2.0f, 2.0f};
         }
      }
      else if (selected.is<block_id>()) {
         const block_id id = selected.get<block_id>();
         const std::optional<uint32> block_index = find_block(world.blocks, id);

         if (block_index) {
            const math::bounding_box bbox =
               get_bounding_box(world.blocks, id.type(), *block_index);

            selection_centre += (bbox.min + bbox.max) / 2.0f;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
   }

   selection_axis_count = max(selection_axis_count, float3{1.0f, 1.0f, 1.0f});

   selection_centre /= selection_axis_count;

   return selection_centre;
}

auto selection_centre_for_env_map(const world& world,
                                  const std::span<const selected_entity> selection,
                                  const object_class_library& object_classes) -> float3
{
   float3 selection_centre = {0.0f, 0.0f, 0.0f};
   float3 selection_axis_count = {0.0f, 0.0f, 0.0f};

   for (const auto& selected : selection) {
      if (selected.is<object_id>()) {
         const object* object =
            find_entity(world.objects, selected.get<object_id>());

         if (object) {
            math::bounding_box bbox =
               object_classes[object->class_handle].model->bounding_box;

            bbox = object->rotation * bbox + object->position;

            selection_centre += ((bbox.max + bbox.min) / 2.0f);
            selection_axis_count += {1.0f, 1.0f, 1.0f};
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

               selection_centre += node.position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
            }
         }
      }
      else if (selected.is<light_id>()) {
         const light* light = find_entity(world.lights, selected.get<light_id>());

         if (light) {
            selection_centre += light->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<region_id>()) {
         const region* region =
            find_entity(world.regions, selected.get<region_id>());

         if (region) {
            selection_centre += region->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<sector_id>()) {
         const sector* sector =
            find_entity(world.sectors, selected.get<sector_id>());

         if (sector) {
            for (auto& point : sector->points) {
               selection_centre += {point.x, 0.0f, point.y};
               selection_axis_count += {1.0f, 0.0f, 1.0f};
            }

            selection_centre += {0.0f, sector->base + (sector->height / 2.0f), 0.0f};
            selection_axis_count += {0.0f, 1.0f, 0.0f};
         }
      }
      else if (selected.is<portal_id>()) {
         const portal* portal =
            find_entity(world.portals, selected.get<portal_id>());

         if (portal) {
            selection_centre += portal->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<hintnode_id>()) {
         const hintnode* hintnode =
            find_entity(world.hintnodes, selected.get<hintnode_id>());

         if (hintnode) {
            selection_centre += hintnode->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<barrier_id>()) {
         const barrier* barrier =
            find_entity(world.barriers, selected.get<barrier_id>());

         if (barrier) {
            selection_centre += barrier->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<planning_hub_id>()) {
         const planning_hub* planning_hub =
            find_entity(world.planning_hubs, selected.get<planning_hub_id>());

         if (planning_hub) {
            selection_centre += planning_hub->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (selected.is<planning_connection_id>()) {
         const planning_connection* planning_connection =
            find_entity(world.planning_connections,
                        selected.get<planning_connection_id>());

         if (planning_connection) {
            selection_centre +=
               world.planning_hubs[planning_connection->start_hub_index].position;
            selection_centre +=
               world.planning_hubs[planning_connection->end_hub_index].position;
            selection_axis_count += {2.0f, 2.0f, 2.0f};
         }
      }
      else if (selected.is<boundary_id>()) {
         const boundary* boundary =
            find_entity(world.boundaries, selected.get<boundary_id>());

         if (boundary) {
            for (const float3& point : boundary->points) {
               selection_centre += point;
               selection_axis_count += 1.0f;
            }
         }
      }
      else if (selected.is<measurement_id>()) {
         const measurement* measurement =
            find_entity(world.measurements, selected.get<measurement_id>());

         if (measurement) {
            selection_centre += measurement->start;
            selection_centre += measurement->end;
            selection_axis_count += {2.0f, 2.0f, 2.0f};
         }
      }
      else if (selected.is<block_id>()) {
         const block_id id = selected.get<block_id>();
         const std::optional<uint32> block_index = find_block(world.blocks, id);

         if (block_index) {
            const math::bounding_box bbox =
               get_bounding_box(world.blocks, id.type(), *block_index);

            selection_centre += (bbox.min + bbox.max) / 2.0f;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
   }

   selection_axis_count = max(selection_axis_count, float3{1.0f, 1.0f, 1.0f});

   selection_centre /= selection_axis_count;

   return selection_centre;
}

}
