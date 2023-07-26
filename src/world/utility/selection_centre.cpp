#include "selection_centre.hpp"
#include "math/vector_funcs.hpp"
#include "world_utilities.hpp"

namespace we::world {

auto selection_centre_for_rotate_around(const world& world,
                                        const std::span<const selected_entity> selection)
   -> float3
{
   float3 selection_centre = {0.0f, 0.0f, 0.0f};
   float3 selection_axis_count = {0.0f, 0.0f, 0.0f};

   for (const auto& selected : selection) {
      if (std::holds_alternative<object_id>(selected)) {
         const object* object =
            find_entity(world.objects, std::get<object_id>(selected));

         if (object) {
            selection_centre += object->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (std::holds_alternative<path_id_node_pair>(selected)) {
         const auto [id, node_index] = std::get<path_id_node_pair>(selected);

         const path* path = find_entity(world.paths, id);

         if (path) {
            const path::node& node = path->nodes[node_index];

            selection_centre += node.position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (std::holds_alternative<light_id>(selected)) {
         const light* light = find_entity(world.lights, std::get<light_id>(selected));

         if (light) {
            selection_centre += light->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (std::holds_alternative<region_id>(selected)) {
         const region* region =
            find_entity(world.regions, std::get<region_id>(selected));

         if (region) {
            selection_centre += region->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (std::holds_alternative<sector_id>(selected)) {
         const sector* sector =
            find_entity(world.sectors, std::get<sector_id>(selected));

         if (sector) {
            for (auto& point : sector->points) {
               selection_centre += {point.x, 0.0f, point.y};
               selection_axis_count += {1.0f, 0.0f, 1.0f};
            }
         }
      }
      else if (std::holds_alternative<portal_id>(selected)) {
         const portal* portal =
            find_entity(world.portals, std::get<portal_id>(selected));

         if (portal) {
            selection_centre += portal->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (std::holds_alternative<hintnode_id>(selected)) {
         const hintnode* hintnode =
            find_entity(world.hintnodes, std::get<hintnode_id>(selected));

         if (hintnode) {
            selection_centre += hintnode->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (std::holds_alternative<barrier_id>(selected)) {
         const barrier* barrier =
            find_entity(world.barriers, std::get<barrier_id>(selected));

         if (barrier) {
            selection_centre += barrier->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (std::holds_alternative<planning_hub_id>(selected)) {
         const planning_hub* planning_hub =
            find_entity(world.planning_hubs, std::get<planning_hub_id>(selected));

         if (planning_hub) {
            selection_centre += planning_hub->position;
            selection_axis_count += {1.0f, 1.0f, 1.0f};
         }
      }
      else if (std::holds_alternative<boundary_id>(selected)) {
         const boundary* boundary =
            find_entity(world.boundaries, std::get<boundary_id>(selected));

         if (boundary) {
            selection_centre += {boundary->position.x, 0.0f, boundary->position.y};
            selection_axis_count += {1.0f, 0.0f, 1.0f};
         }
      }
   }

   selection_axis_count = max(selection_axis_count, float3{1.0f, 1.0f, 1.0f});

   selection_centre /= selection_axis_count;

   return selection_centre;
}

}
