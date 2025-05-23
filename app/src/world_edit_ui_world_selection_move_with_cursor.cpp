#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/bounding_box.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/raycast.hpp"
#include "world/utility/raycast_terrain.hpp"
#include "world/utility/selection_bbox.hpp"
#include "world/utility/snapping.hpp"
#include "world/utility/terrain_cut.hpp"
#include "world/utility/world_utilities.hpp"

namespace we {

void world_edit::ui_show_world_selection_move_with_cursor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = _selection_edit_tool == selection_edit_tool::move_with_cursor;

   if (ImGui::Begin("Move Selection##With Cursor", &open,
                    ImGuiWindowFlags_AlwaysAutoResize)) {
      const world::active_entity_types raycast_mask = _selection_cursor_move_hit_mask;

      graphics::camera_ray ray =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      const auto filter_entity = [&](const auto& entity) noexcept {
         for (const auto& selected : _interaction_targets.selection) {
            if (world::selected_entity{entity.id} == selected) return false;
         }

         return true;
      };

      float cursor_distance = std::numeric_limits<float>::max();

      if (raycast_mask.objects) {
         if (std::optional<world::raycast_result<world::object>> hit =
                world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                               _world.objects, _object_classes, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.lights) {
         if (std::optional<world::raycast_result<world::light>> hit =
                world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                               _world.lights, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.paths) {
         const auto path_filter = [&](const world::path& path, uint32 node_index) noexcept {
            for (const auto& selected : _interaction_targets.selection) {
               if (selected.is<world::path_id_node_mask>()) {
                  const auto& [id, node_mask] =
                     selected.get<world::path_id_node_mask>();

                  if (id == path.id and node_mask[node_index]) return false;
               }
            }

            return true;
         };

         if (std::optional<world::raycast_result<world::path>> hit =
                world::raycast(ray.origin, ray.direction,
                               _world_layers_hit_mask, _world.paths,
                               _settings.graphics.path_node_size, path_filter);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.regions) {
         if (std::optional<world::raycast_result<world::region>> hit =
                world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                               _world.regions, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.sectors) {
         if (std::optional<world::raycast_result<world::sector>> hit =
                world::raycast(ray.origin, ray.direction, _world.sectors, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.portals) {
         if (std::optional<world::raycast_result<world::portal>> hit =
                world::raycast(ray.origin, ray.direction, _world.portals, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.hintnodes) {
         if (std::optional<world::raycast_result<world::hintnode>> hit =
                world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                               _world.hintnodes, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.barriers) {
         if (std::optional<world::raycast_result<world::barrier>> hit =
                world::raycast(ray.origin, ray.direction, _world.barriers,
                               _settings.graphics.barrier_height, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.planning_hubs) {
         if (std::optional<world::raycast_result<world::planning_hub>> hit =
                world::raycast(ray.origin, ray.direction, _world.planning_hubs,
                               _settings.graphics.planning_hub_height, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.planning_connections) {
         if (std::optional<world::raycast_result<world::planning_connection>> hit =
                world::raycast(ray.origin, ray.direction,
                               _world.planning_connections, _world.planning_hubs,
                               _settings.graphics.planning_connection_height,
                               filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.boundaries) {
         if (std::optional<world::raycast_result<world::boundary>> hit =
                world::raycast(ray.origin, ray.direction, _world.boundaries,
                               _settings.graphics.boundary_height, filter_entity);
             hit) {
            cursor_distance = std::min(cursor_distance, hit->distance);
         }
      }

      if (raycast_mask.terrain and _world.terrain.active_flags.terrain) {
         if (auto hit = world::raycast(ray.origin, ray.direction, _world.terrain); hit) {
            if (*hit < cursor_distance) {
               if (not raycast_mask.objects or
                   not world::point_inside_terrain_cut(ray.origin + ray.direction * *hit,
                                                       ray.direction, _world_layers_hit_mask,
                                                       _world.objects,
                                                       _object_classes)) {
                  _interaction_targets.hovered_entity = std::nullopt;
                  cursor_distance = *hit;
               }
            }
         }
      }

      if (cursor_distance == std::numeric_limits<float>::max()) {
         if (float hit = -(dot(ray.origin, float3{0.0f, 1.0f, 0.0f}) - _editor_floor_height) /
                         dot(ray.direction, float3{0.0f, 1.0f, 0.0f});
             hit > 0.0f) {
            cursor_distance = hit;
         }
      }

      const world::selection_metrics metrics =
         selection_metrics_for_move(_world, _interaction_targets.selection,
                                    _object_classes);

      float3 selection_centreWS = metrics.centreWS;
      float3 cursor_positionWS = cursor_distance != std::numeric_limits<float>::max()
                                    ? ray.origin + ray.direction * cursor_distance
                                    : float3{0.0f, 0.0f, 0.0f};

      if (_selection_cursor_move_ground_with_bbox) {
         const float ground_distance = metrics.bboxWS.min.y - selection_centreWS.y;

         cursor_positionWS.y -= ground_distance;
      }

      if (_selection_cursor_move_align_cursor) {
         cursor_positionWS.x =
            std::round(cursor_positionWS.x / _editor_grid_size) * _editor_grid_size;
         cursor_positionWS.z =
            std::round(cursor_positionWS.z / _editor_grid_size) * _editor_grid_size;
      }

      if (_selection_cursor_move_snap_cursor) {
         math::bounding_box bboxOS;
         quaternion world_from_object;

         if (_interaction_targets.selection.size() == 1 and
             _interaction_targets.selection[0].is<world::object_id>()) {
            if (world::object* object =
                   world::find_entity(_world.objects,
                                      _interaction_targets.selection[0].get<world::object_id>());
                object) {
               bboxOS = _object_classes[object->class_handle].model->bounding_box;
               world_from_object = object->rotation;
            }
         }
         else {
            bboxOS = {.min = metrics.bboxWS.min - selection_centreWS,
                      .max = metrics.bboxWS.max - selection_centreWS};
         }

         cursor_positionWS = world::get_snapped_position_filtered(
            {
               .rotation = world_from_object,
               .positionWS = cursor_positionWS,
               .bboxOS = bboxOS,
            },
            _world.objects, _interaction_targets.selection,
            _selection_cursor_move_snap_distance,
            {
               .snap_to_corners = _selection_cursor_move_snap_to_corners,
               .snap_to_edge_midpoints = _selection_cursor_move_snap_to_edge_midpoints,
               .snap_to_face_midpoints = _selection_cursor_move_snap_to_face_midpoints,
            },
            _world_layers_draw_mask, _object_classes, _tool_visualizers,
            {
               .snapped = _settings.graphics.snapping_snapped_color,
               .corner = _settings.graphics.snapping_corner_color,
               .edge = _settings.graphics.snapping_edge_color,
               .face = _settings.graphics.snapping_face_color,
            });
      }

      edits::bundle_vector bundled_edits;

      const bool lock_x = _selection_cursor_move_lock_x_axis;
      const bool lock_y = _selection_cursor_move_lock_y_axis;
      const bool lock_z = _selection_cursor_move_lock_z_axis;

      const auto update_position = [&](float3* position) {
         assert(position);

         float3 new_position = *position - selection_centreWS + cursor_positionWS;

         if (lock_x) new_position.x = position->x;
         if (lock_y) new_position.y = position->y;
         if (lock_z) new_position.z = position->z;

         bundled_edits.push_back(edits::make_set_value(position, new_position));
      };

      for (const auto& selected : _interaction_targets.selection) {
         if (selected.is<world::object_id>()) {
            world::object* object =
               world::find_entity(_world.objects, selected.get<world::object_id>());

            if (object) update_position(&object->position);
         }
         else if (selected.is<world::path_id_node_mask>()) {
            const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

            world::path* path = world::find_entity(_world.paths, id);

            if (path) {
               const std::size_t node_count =
                  std::min(path->nodes.size(), world::max_path_nodes);

               for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                  if (not node_mask[node_index]) continue;

                  const world::path::node& node = path->nodes[node_index];

                  float3 new_position =
                     node.position - selection_centreWS + cursor_positionWS;

                  // clang-format off

                  if (lock_x) new_position.x = path->nodes[node_index].position.x;
                  if (lock_y) new_position.y = path->nodes[node_index].position.y;
                  if (lock_z) new_position.z = path->nodes[node_index].position.z;

                  // clang-format on

                  bundled_edits.push_back(
                     edits::make_set_vector_value(&path->nodes, node_index,
                                                  &world::path::node::position,
                                                  new_position));
               }
            }
         }
         else if (selected.is<world::light_id>()) {
            world::light* light =
               world::find_entity(_world.lights, selected.get<world::light_id>());

            if (light) update_position(&light->position);
         }
         else if (selected.is<world::region_id>()) {
            world::region* region =
               world::find_entity(_world.regions, selected.get<world::region_id>());

            if (region) update_position(&region->position);
         }
         else if (selected.is<world::sector_id>()) {
            world::sector* sector =
               world::find_entity(_world.sectors, selected.get<world::sector_id>());

            if (sector) {
               std::vector<float2> new_points = sector->points;

               for (std::size_t i = 0; i < new_points.size(); ++i) {
                  new_points[i] -=
                     float2{selection_centreWS.x, selection_centreWS.z};
                  new_points[i] += float2{cursor_positionWS.x, cursor_positionWS.z};

                  if (lock_x) new_points[i].x = sector->points[i].x;
                  if (lock_z) new_points[i].y = sector->points[i].y;
               }

               bundled_edits.push_back(
                  edits::make_set_value(&sector->points, std::move(new_points)));

               if (not lock_y) {
                  bundled_edits.push_back(
                     edits::make_set_value(&sector->base,
                                           (sector->base - selection_centreWS.y) +
                                              cursor_positionWS.y));
               }
            }
         }
         else if (selected.is<world::portal_id>()) {
            world::portal* portal =
               world::find_entity(_world.portals, selected.get<world::portal_id>());

            if (portal) update_position(&portal->position);
         }
         else if (selected.is<world::hintnode_id>()) {
            world::hintnode* hintnode =
               world::find_entity(_world.hintnodes,
                                  selected.get<world::hintnode_id>());

            if (hintnode) update_position(&hintnode->position);
         }
         else if (selected.is<world::barrier_id>()) {
            world::barrier* barrier =
               world::find_entity(_world.barriers, selected.get<world::barrier_id>());

            if (barrier) update_position(&barrier->position);
         }
         else if (selected.is<world::planning_hub_id>()) {
            world::planning_hub* planning_hub =
               world::find_entity(_world.planning_hubs,
                                  selected.get<world::planning_hub_id>());

            if (planning_hub) update_position(&planning_hub->position);
         }
         else if (selected.is<world::boundary_id>()) {
            world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  selected.get<world::boundary_id>());

            if (boundary) update_position(&boundary->position);
         }
         else if (selected.is<world::measurement_id>()) {
            world::measurement* measurement =
               world::find_entity(_world.measurements,
                                  selected.get<world::measurement_id>());

            if (measurement) {
               update_position(&measurement->start);
               update_position(&measurement->end);
            }
         }
      }

      if (bundled_edits.size() == 1) {
         _edit_stack_world.apply(std::move(bundled_edits.back()), _edit_context);
      }
      else if (not bundled_edits.empty()) {
         _edit_stack_world.apply(edits::make_bundle(std::move(bundled_edits)),
                                 _edit_context);
      }

      ImGui::LabelText("Cursor Position", "X:%.3f Y:%.3f Z:%.3f",
                       cursor_positionWS.x, cursor_positionWS.y,
                       cursor_positionWS.z);

      ImGui::Checkbox("Ground with Selection BBOX",
                      &_selection_cursor_move_ground_with_bbox);

      ImGui::Checkbox("Align Cursor to Grid", &_selection_cursor_move_align_cursor);

      ImGui::Checkbox("Snapping", &_selection_cursor_move_snap_cursor);

      if (_selection_cursor_move_snap_cursor) {
         ImGui::SeparatorText("Snapping Config");

         if (ImGui::BeginTable("Snapping Config", 3,
                               ImGuiTableFlags_NoSavedSettings |
                                  ImGuiTableFlags_SizingStretchSame)) {

            ImGui::TableNextColumn();
            ImGui::Selectable("Corners", &_selection_cursor_move_snap_to_corners);
            ImGui::SetItemTooltip("Snap with bounding box corners.");

            ImGui::TableNextColumn();
            ImGui::Selectable("Edges", &_selection_cursor_move_snap_to_edge_midpoints);
            ImGui::SetItemTooltip("Snap with bounding box edge midpoints.");

            ImGui::TableNextColumn();
            ImGui::Selectable("Faces", &_selection_cursor_move_snap_to_face_midpoints);
            ImGui::SetItemTooltip(
               "Snap with bounding box top and bottom face midpoints.");

            ImGui::EndTable();
         }

         ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                              ImGui::GetStyle().CellPadding.y);

         ImGui::DragFloat("Snap Distance", &_selection_cursor_move_snap_distance,
                          0.1f, 0.0f, 1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      }

      ImGui::SeparatorText("Cursor Collision");

      world::active_entity_types& cursor_mask = _selection_cursor_move_hit_mask;

      if (bool hit = cursor_mask.objects; ImGui::Checkbox("Objects", &hit)) {
         cursor_mask.objects = hit;
      }

      if (bool hit = cursor_mask.lights; ImGui::Checkbox("Lights", &hit)) {
         cursor_mask.lights = hit;
      }

      if (bool hit = cursor_mask.paths; ImGui::Checkbox("Paths", &hit)) {
         cursor_mask.paths = hit;
      }

      if (bool hit = cursor_mask.regions; ImGui::Checkbox("Regions", &hit)) {
         cursor_mask.regions = hit;
      }

      if (bool hit = cursor_mask.sectors; ImGui::Checkbox("Sectors", &hit)) {
         cursor_mask.sectors = hit;
      }

      if (bool hit = cursor_mask.portals; ImGui::Checkbox("Portals", &hit)) {
         cursor_mask.portals = hit;
      }

      if (bool hit = cursor_mask.hintnodes; ImGui::Checkbox("Hintnodes", &hit)) {
         cursor_mask.hintnodes = hit;
      }

      if (bool hit = cursor_mask.barriers; ImGui::Checkbox("Barriers", &hit)) {
         cursor_mask.barriers = hit;
      }

      if (bool hit = cursor_mask.planning_hubs; ImGui::Checkbox("AI Planning", &hit)) {
         cursor_mask.planning_hubs = hit;
         cursor_mask.planning_connections = hit;
      }

      if (bool hit = cursor_mask.boundaries; ImGui::Checkbox("Boundaries", &hit)) {
         cursor_mask.boundaries = hit;
      }

      if (bool hit = cursor_mask.terrain; ImGui::Checkbox("Terrain", &hit)) {
         cursor_mask.terrain = hit;
      }

      ImGui::SeparatorText("Locked Position");

      if (ImGui::BeginTable("Locked Position", 3,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {

         ImGui::TableNextColumn();
         ImGui::Selectable("X", &_selection_cursor_move_lock_x_axis);
         ImGui::TableNextColumn();
         ImGui::Selectable("Y", &_selection_cursor_move_lock_y_axis);
         ImGui::TableNextColumn();
         ImGui::Selectable("Z", &_selection_cursor_move_lock_z_axis);

         ImGui::EndTable();
      }

      ImGui::Separator();

      ImGui::Text("Click in world to end move.");

      if (not open) {
         _edit_stack_world.close_last();
         _selection_edit_tool = selection_edit_tool::none;
      }
   }

   ImGui::End();

   if (_selection_cursor_move_rotate_forward or _selection_cursor_move_rotate_back) {
      const float3 centreWS =
         selection_metrics_for_move(_world, _interaction_targets.selection, _object_classes)
            .centreWS;

      const float rotate_step =
         _selection_cursor_move_rotate_forward ? 0.2617994f : -0.2617994f;
      const quaternion rotation = make_quat_from_euler({0.0f, rotate_step, 0.0f});

      edits::bundle_vector bundled_edits;

      for (const auto& selected : _interaction_targets.selection) {
         if (selected.is<world::object_id>()) {
            world::object* object =
               world::find_entity(_world.objects, selected.get<world::object_id>());

            if (object) {
               bundled_edits.push_back(
                  edits::make_set_value(&object->rotation, rotation * object->rotation));
               bundled_edits.push_back(
                  edits::make_set_value(&object->position,
                                        (rotation * (object->position - centreWS)) +
                                           centreWS));
            }
         }
         else if (selected.is<world::light_id>()) {
            world::light* light =
               world::find_entity(_world.lights, selected.get<world::light_id>());

            if (light) {
               bundled_edits.push_back(
                  edits::make_set_value(&light->rotation, rotation * light->rotation));
               bundled_edits.push_back(
                  edits::make_set_value(&light->position,
                                        (rotation * (light->position - centreWS)) +
                                           centreWS));

               if (world::is_region_light(*light)) {
                  bundled_edits.push_back(
                     edits::make_set_value(&light->region_rotation,
                                           rotation * light->region_rotation));
               }
            }
         }
         else if (selected.is<world::path_id_node_mask>()) {
            const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

            world::path* path = world::find_entity(_world.paths, id);

            if (path) {
               const std::size_t node_count =
                  std::min(path->nodes.size(), world::max_path_nodes);

               for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                  if (not node_mask[node_index]) continue;

                  const world::path::node& node = path->nodes[node_index];

                  bundled_edits.push_back(
                     edits::make_set_vector_value(&path->nodes, node_index,
                                                  &world::path::node::rotation,
                                                  rotation * node.rotation));
                  bundled_edits.push_back(edits::make_set_vector_value(
                     &path->nodes, node_index, &world::path::node::position,
                     (rotation * (node.position - centreWS)) + centreWS));
               }
            }
         }
         else if (selected.is<world::region_id>()) {
            world::region* region =
               world::find_entity(_world.regions, selected.get<world::region_id>());

            if (region) {
               bundled_edits.push_back(
                  edits::make_set_value(&region->rotation, rotation * region->rotation));
               bundled_edits.push_back(
                  edits::make_set_value(&region->position,
                                        (rotation * (region->position - centreWS)) +
                                           centreWS));
            }
         }
         else if (selected.is<world::sector_id>()) {
            world::sector* sector =
               world::find_entity(_world.sectors, selected.get<world::sector_id>());

            if (sector) {
               float2 sector_centre = {0.0f, 0.0f};

               for (const auto& point : sector->points) {
                  sector_centre += (point);
               }

               sector_centre /= static_cast<float>(sector->points.size());

               const float3 rotated_sector_centre =
                  (rotation *
                   (float3{sector_centre.x, 0.0f, sector_centre.y} - centreWS)) +
                  centreWS;

               std::vector<float2> new_points = sector->points;

               for (auto& point : new_points) {
                  const float3 rotated_point =
                     rotation * float3{point.x - sector_centre.x, 0.0f,
                                       point.y - sector_centre.y};

                  point = float2{rotated_point.x, rotated_point.z} +
                          float2{rotated_sector_centre.x, rotated_sector_centre.z};
               }

               bundled_edits.push_back(
                  edits::make_set_value(&sector->points, std::move(new_points)));
            }
         }
         else if (selected.is<world::portal_id>()) {
            world::portal* portal =
               world::find_entity(_world.portals, selected.get<world::portal_id>());

            if (portal) {
               bundled_edits.push_back(
                  edits::make_set_value(&portal->rotation, rotation * portal->rotation));
               bundled_edits.push_back(
                  edits::make_set_value(&portal->position,
                                        (rotation * (portal->position - centreWS)) +
                                           centreWS));
            }
         }
         else if (selected.is<world::hintnode_id>()) {
            world::hintnode* hintnode =
               world::find_entity(_world.hintnodes,
                                  selected.get<world::hintnode_id>());

            if (hintnode) {
               bundled_edits.push_back(
                  edits::make_set_value(&hintnode->rotation,
                                        rotation * hintnode->rotation));
               bundled_edits.push_back(
                  edits::make_set_value(&hintnode->position,
                                        (rotation * (hintnode->position - centreWS)) +
                                           centreWS));
            }
         }
         else if (selected.is<world::barrier_id>()) {
            world::barrier* barrier =
               world::find_entity(_world.barriers, selected.get<world::barrier_id>());

            if (barrier) {
               bundled_edits.push_back(
                  edits::make_set_value(&barrier->rotation_angle,
                                        barrier->rotation_angle + rotate_step));
               bundled_edits.push_back(
                  edits::make_set_value(&barrier->position,
                                        (rotation * (barrier->position - centreWS)) +
                                           centreWS));
            }
         }
         else if (selected.is<world::planning_hub_id>()) {
            world::planning_hub* hub =
               world::find_entity(_world.planning_hubs,
                                  selected.get<world::planning_hub_id>());

            if (hub) {
               bundled_edits.push_back(
                  edits::make_set_value(&hub->position,
                                        (rotation * (hub->position - centreWS)) +
                                           centreWS));
            }
         }
         else if (selected.is<world::boundary_id>()) {
            world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  selected.get<world::boundary_id>());

            if (boundary) {
               bundled_edits.push_back(
                  edits::make_set_value(&boundary->position,
                                        (rotation * (boundary->position - centreWS)) +
                                           centreWS));
            }
         }
         else if (selected.is<world::measurement_id>()) {
            world::measurement* measurement =
               world::find_entity(_world.measurements,
                                  selected.get<world::measurement_id>());

            if (measurement) {
               bundled_edits.push_back(
                  edits::make_set_value(&measurement->start,
                                        (rotation * (measurement->start - centreWS)) +
                                           centreWS));
               bundled_edits.push_back(
                  edits::make_set_value(&measurement->end,
                                        (rotation * (measurement->end - centreWS)) +
                                           centreWS));
            }
         }
      }

      if (bundled_edits.size() == 1) {
         _edit_stack_world.apply(std::move(bundled_edits.back()), _edit_context,
                                 {.transparent = true});
      }
      else if (not bundled_edits.empty()) {
         _edit_stack_world.apply(edits::make_bundle(std::move(bundled_edits)),
                                 _edit_context, {.transparent = true});
      }

      _selection_cursor_move_rotate_back = false;
      _selection_cursor_move_rotate_forward = false;
   }

   if (open and _hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Move Selection with Cursor");

      ImGui::Text("Toggle Cursor Alignment");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing (Move Selection with Cursor)",
                                "Toggle Cursor Alignment")));

      ImGui::Text("Toggle Cursor Snapping");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing (Move Selection with Cursor)",
                                "Toggle Cursor Snapping")));

      ImGui::Text("Lock X Axis");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing (Move Selection with Cursor)",
                                "Lock X Axis")));

      ImGui::Text("Lock Y Axis");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing (Move Selection with Cursor)",
                                "Lock Y Axis")));

      ImGui::Text("Lock Z Axis");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing (Move Selection with Cursor)",
                                "Lock Z Axis")));

      ImGui::Text("Rotate Selection Forward");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing (Move Selection with Cursor)",
                                "Rotate Selection Forward")));

      ImGui::Text("Rotate Selection Back");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing (Move Selection with Cursor)",
                                "Rotate Selection Back")));

      ImGui::End();
   }
}
}