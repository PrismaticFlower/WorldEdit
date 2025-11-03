#include "double_click_select.hpp"

#include "intersects_frustum.hpp"
#include "is_similar.hpp"
#include "world_utilities.hpp"

#include "../blocks/utility/drag_select.hpp"

namespace we::world {

void double_click_select(const interaction_target& hovered_entity, const world& world,
                         const object_class_library& object_classes,
                         const blocks_custom_mesh_bvh_library& bvh_library,
                         const frustum& frustumWS, double_click_select_op op,
                         selection& selection,
                         const double_click_settings& settings) noexcept
{
   if (hovered_entity.is<object_id>()) {
      const object* hovered_object =
         find_entity(world.objects, hovered_entity.get<object_id>());

      if (not hovered_object) return;

      for (const object& object : world.objects) {
         if (object.hidden) continue;
         if (not is_similar(object, *hovered_object)) continue;

         if (intersects(frustumWS, object, object_classes)) {
            if (op == double_click_select_op::remove) {
               selection.remove(object.id);
            }
            else {
               selection.add(object.id);
            }
         }
      }
   }
   else if (hovered_entity.is<light_id>()) {
      const light* hovered_light =
         find_entity(world.lights, hovered_entity.get<light_id>());

      if (not hovered_light) return;

      for (const light& light : world.lights) {
         if (light.hidden) continue;
         if (not is_similar(light, *hovered_light)) continue;

         if (intersects(frustumWS, light)) {
            if (op == double_click_select_op::remove) {
               selection.remove(light.id);
            }
            else {
               selection.add(light.id);
            }
         }
      }
   }
   else if (hovered_entity.is<path_id_node_mask>()) {
      const path* hovered_path =
         find_entity(world.paths, hovered_entity.get<path_id_node_mask>().id);

      if (not hovered_path) return;

      for (uint32 i = 0; i < hovered_path->nodes.size(); ++i) {
         if (intersects(frustumWS, hovered_path->nodes[i].position,
                        settings.path_node_radius)) {
            if (op == double_click_select_op::remove) {
               selection.remove(make_path_id_node_mask(hovered_path->id, i));
            }
            else {
               selection.add(make_path_id_node_mask(hovered_path->id, i));
            }
         }
      }
   }
   else if (hovered_entity.is<region_id>()) {
      const region* hovered_region =
         find_entity(world.regions, hovered_entity.get<region_id>());

      if (not hovered_region) return;

      for (const region& region : world.regions) {
         if (region.hidden) continue;
         if (not is_similar(region, *hovered_region)) continue;

         if (intersects(frustumWS, region)) {
            if (op == double_click_select_op::remove) {
               selection.remove(region.id);
            }
            else {
               selection.add(region.id);
            }
         }
      }
   }
   else if (hovered_entity.is<sector_id>()) {
      const sector* hovered_sector =
         find_entity(world.sectors, hovered_entity.get<sector_id>());

      if (not hovered_sector) return;

      for (const sector& sector : world.sectors) {
         if (sector.hidden) continue;

         if (intersects(frustumWS, sector)) {
            if (op == double_click_select_op::remove) {
               selection.remove(sector.id);
            }
            else {
               selection.add(sector.id);
            }
         }
      }
   }
   else if (hovered_entity.is<portal_id>()) {
      const portal* hovered_portal =
         find_entity(world.portals, hovered_entity.get<portal_id>());

      if (not hovered_portal) return;

      for (const portal& portal : world.portals) {
         if (portal.hidden) continue;
         if (not is_similar(portal, *hovered_portal)) continue;

         if (intersects(frustumWS, portal)) {
            if (op == double_click_select_op::remove) {
               selection.remove(portal.id);
            }
            else {
               selection.add(portal.id);
            }
         }
      }
   }
   else if (hovered_entity.is<barrier_id>()) {
      const barrier* hovered_barrier =
         find_entity(world.barriers, hovered_entity.get<barrier_id>());

      if (not hovered_barrier) return;

      for (const barrier& barrier : world.barriers) {
         if (barrier.hidden) continue;
         if (not is_similar(barrier, *hovered_barrier)) continue;

         if (intersects(frustumWS, barrier, settings.barrier_visualizer_height)) {
            if (op == double_click_select_op::remove) {
               selection.remove(barrier.id);
            }
            else {
               selection.add(barrier.id);
            }
         }
      }
   }
   else if (hovered_entity.is<hintnode_id>()) {
      const hintnode* hovered_hintnode =
         find_entity(world.hintnodes, hovered_entity.get<hintnode_id>());

      if (not hovered_hintnode) return;

      for (const hintnode& hintnode : world.hintnodes) {
         if (hintnode.hidden) continue;
         if (not is_similar(hintnode, *hovered_hintnode)) continue;

         if (intersects(frustumWS, hintnode)) {
            if (op == double_click_select_op::remove) {
               selection.remove(hintnode.id);
            }
            else {
               selection.add(hintnode.id);
            }
         }
      }
   }
   else if (hovered_entity.is<planning_hub_id>()) {
      const planning_hub* hovered_boundary =
         find_entity(world.planning_hubs, hovered_entity.get<planning_hub_id>());

      if (not hovered_boundary) return;

      for (const planning_hub& planning_hub : world.planning_hubs) {
         if (planning_hub.hidden) continue;

         if (intersects(frustumWS, planning_hub, settings.hub_visualizer_height)) {
            if (op == double_click_select_op::remove) {
               selection.remove(planning_hub.id);
            }
            else {
               selection.add(planning_hub.id);
            }
         }
      }
   }
   else if (hovered_entity.is<planning_connection_id>()) {
      const planning_connection* hovered_connection =
         find_entity(world.planning_connections,
                     hovered_entity.get<planning_connection_id>());

      if (not hovered_connection) return;

      for (const planning_connection& planning_connection : world.planning_connections) {
         if (planning_connection.hidden) continue;
         if (not is_similar(planning_connection, *hovered_connection)) continue;

         if (intersects(frustumWS, planning_connection, world.planning_hubs,
                        settings.connection_visualizer_height)) {
            if (op == double_click_select_op::remove) {
               selection.remove(planning_connection.id);
            }
            else {
               selection.add(planning_connection.id);
            }
         }
      }
   }
   else if (hovered_entity.is<boundary_id>()) {
      const boundary* hovered_boundary =
         find_entity(world.boundaries, hovered_entity.get<boundary_id>());

      if (not hovered_boundary) return;

      for (const boundary& boundary : world.boundaries) {
         if (boundary.hidden) continue;

         if (intersects(frustumWS, boundary, settings.boundary_visualizer_height)) {
            if (op == double_click_select_op::remove) {
               selection.remove(boundary.id);
            }
            else {
               selection.add(boundary.id);
            }
         }
      }
   }
   else if (hovered_entity.is<measurement_id>()) {
      const measurement* hovered_measurement =
         find_entity(world.measurements, hovered_entity.get<measurement_id>());

      if (not hovered_measurement) return;

      for (const measurement& measurement : world.measurements) {
         if (measurement.hidden) continue;

         if (intersects(frustumWS, measurement)) {
            if (op == double_click_select_op::remove) {
               selection.remove(measurement.id);
            }
            else {
               selection.add(measurement.id);
            }
         }
      }
   }
   else if (hovered_entity.is<block_id>()) {
      drag_select(world.blocks, bvh_library, frustumWS,
                  op == double_click_select_op::add ? block_drag_select_op::add
                                                    : block_drag_select_op::remove,
                  selection);
   }
}

}