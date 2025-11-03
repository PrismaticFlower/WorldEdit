#include "drag_select.hpp"

#include "intersects_frustum.hpp"

#include "../blocks/utility/drag_select.hpp"

namespace we::world {

void drag_select(const world& world, const active_entity_types active_entities,
                 const active_layers active_layers,
                 const object_class_library& object_classes,
                 const blocks_custom_mesh_bvh_library& bvh_library,
                 const frustum& frustumWS, select_op op, selection& selection,
                 const select_settings& settings) noexcept
{
   if (active_entities.objects) {
      for (const object& object : world.objects) {
         if (not active_layers[object.layer] or object.hidden) {
            continue;
         }

         if (intersects(frustumWS, object, object_classes)) {
            if (op == select_op::remove) {
               selection.remove(object.id);
            }
            else {
               selection.add(object.id);
            }
         }
      }
   }

   if (active_entities.lights) {
      for (const light& light : world.lights) {
         if (not active_layers[light.layer] or light.hidden) {
            continue;
         }

         if (intersects(frustumWS, light)) {
            if (op == select_op::remove) {
               selection.remove(light.id);
            }
            else {
               selection.add(light.id);
            }
         }
      }
   }

   if (active_entities.paths) {
      for (const path& path : world.paths) {
         if (not active_layers[path.layer] or path.hidden) {
            continue;
         }

         for (uint32 i = 0; i < path.nodes.size(); ++i) {
            if (intersects(frustumWS, path.nodes[i].position, settings.path_node_radius)) {
               if (op == select_op::remove) {
                  selection.remove(make_path_id_node_mask(path.id, i));
               }
               else {
                  selection.add(make_path_id_node_mask(path.id, i));
               }
            }
         }
      }
   }

   if (active_entities.regions) {
      for (const region& region : world.regions) {
         if (not active_layers[region.layer] or region.hidden) {
            continue;
         }

         if (intersects(frustumWS, region)) {
            if (op == select_op::remove) {
               selection.remove(region.id);
            }
            else {
               selection.add(region.id);
            }
         }
      }
   }

   if (active_entities.sectors) {
      for (const sector& sector : world.sectors) {
         if (sector.hidden) continue;

         if (intersects(frustumWS, sector)) {
            if (op == select_op::remove) {
               selection.remove(sector.id);
            }
            else {
               selection.add(sector.id);
            }
         }
      }
   }

   if (active_entities.portals) {
      for (const portal& portal : world.portals) {
         if (portal.hidden) continue;

         if (intersects(frustumWS, portal)) {
            if (op == select_op::remove) {
               selection.remove(portal.id);
            }
            else {
               selection.add(portal.id);
            }
         }
      }
   }

   if (active_entities.hintnodes) {
      for (const hintnode& hintnode : world.hintnodes) {
         if (not active_layers[hintnode.layer] or hintnode.hidden) {
            continue;
         }

         if (intersects(frustumWS, hintnode)) {
            if (op == select_op::remove) {
               selection.remove(hintnode.id);
            }
            else {
               selection.add(hintnode.id);
            }
         }
      }
   }

   if (active_entities.barriers) {
      for (const barrier& barrier : world.barriers) {
         if (barrier.hidden) continue;

         if (intersects(frustumWS, barrier, settings.barrier_visualizer_height)) {
            if (op == select_op::remove) {
               selection.remove(barrier.id);
            }
            else {
               selection.add(barrier.id);
            }
         }
      }
   }

   if (active_entities.planning_hubs) {
      for (const planning_hub& hub : world.planning_hubs) {
         if (hub.hidden) continue;

         if (intersects(frustumWS, hub, settings.hub_visualizer_height)) {
            if (op == select_op::remove) {
               selection.remove(hub.id);
            }
            else {
               selection.add(hub.id);
            }
         }
      }
   }

   if (active_entities.planning_connections) {
      for (const planning_connection& connection : world.planning_connections) {
         if (connection.hidden) continue;

         if (intersects(frustumWS, connection, world.planning_hubs,
                        settings.connection_visualizer_height)) {
            if (op == select_op::remove) {
               selection.remove(connection.id);
            }
            else {
               selection.add(connection.id);
            }
         }
      }
   }

   if (active_entities.boundaries) {
      for (const boundary& boundary : world.boundaries) {
         if (boundary.hidden) continue;

         if (intersects(frustumWS, boundary, settings.barrier_visualizer_height)) {
            if (op == select_op::remove) {
               selection.remove(boundary.id);
            }
            else {
               selection.add(boundary.id);
            }
         }
      }
   }

   if (active_entities.measurements) {
      for (const measurement& measurement : world.measurements) {
         if (measurement.hidden) continue;

         if (intersects(frustumWS, measurement)) {
            if (op == select_op::remove) {
               selection.remove(measurement.id);
            }
            else {
               selection.add(measurement.id);
            }
         }
      }
   }

   if (active_entities.blocks) {
      drag_select(world.blocks, active_layers, bvh_library, frustumWS,
                  op == select_op::remove ? block_drag_select_op::remove
                                          : block_drag_select_op::add,
                  selection);
   }
}

}