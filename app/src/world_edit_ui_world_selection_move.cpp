#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/world_utilities.hpp"

namespace we {

void world_edit::ui_show_world_selection_move() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = _selection_edit_tool == selection_edit_tool::move;

   if (ImGui::Begin("Move Selection", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      float3 selection_centre = {0.0f, 0.0f, 0.0f};
      float3 selection_axis_count = {0.0f, 0.0f, 0.0f};
      quaternion gizmo_rotation;

      for (const auto& selected : _interaction_targets.selection) {
         if (std::holds_alternative<world::object_id>(selected)) {
            const world::object* object =
               world::find_entity(_world.objects, std::get<world::object_id>(selected));

            if (object) {
               if (_gizmo_object_placement == gizmo_object_placement::position) {
                  selection_centre += object->position;
                  selection_axis_count += {1.0f, 1.0f, 1.0f};
               }
               else {
                  math::bounding_box bbox =
                     _object_classes[object->class_name].model->bounding_box;

                  bbox = object->rotation * bbox + object->position;

                  selection_centre += ((bbox.max + bbox.min) / 2.0f);
                  selection_axis_count += {1.0f, 1.0f, 1.0f};
               }

               gizmo_rotation = object->rotation;
            }
         }
         else if (std::holds_alternative<world::path_id_node_mask>(selected)) {
            const auto& [id, node_mask] = std::get<world::path_id_node_mask>(selected);

            const world::path* path = world::find_entity(_world.paths, id);

            if (path) {
               const std::size_t node_count =
                  std::min(path->nodes.size(), world::max_path_nodes);

               for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                  if (not node_mask[node_index]) continue;

                  const world::path::node& node = path->nodes[node_index];

                  selection_centre += node.position;
                  selection_axis_count += {1.0f, 1.0f, 1.0f};
                  gizmo_rotation = node.rotation;
               }
            }
         }
         else if (std::holds_alternative<world::light_id>(selected)) {
            const world::light* light =
               world::find_entity(_world.lights, std::get<world::light_id>(selected));

            if (light) {
               selection_centre += light->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation = light->rotation;
            }
         }
         else if (std::holds_alternative<world::region_id>(selected)) {
            const world::region* region =
               world::find_entity(_world.regions, std::get<world::region_id>(selected));

            if (region) {
               selection_centre += region->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation = region->rotation;
            }
         }
         else if (std::holds_alternative<world::sector_id>(selected)) {
            const world::sector* sector =
               world::find_entity(_world.sectors, std::get<world::sector_id>(selected));

            if (sector) {
               for (auto& point : sector->points) {
                  selection_centre += {point.x, 0.0f, point.y};
                  selection_axis_count += {1.0f, 0.0f, 1.0f};
               }

               selection_centre +=
                  {0.0f, sector->base + (sector->height / 2.0f), 0.0f};
               selection_axis_count += {0.0f, 1.0f, 0.0f};
            }
         }
         else if (std::holds_alternative<world::portal_id>(selected)) {
            const world::portal* portal =
               world::find_entity(_world.portals, std::get<world::portal_id>(selected));

            if (portal) {
               selection_centre += portal->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation = portal->rotation;
            }
         }
         else if (std::holds_alternative<world::hintnode_id>(selected)) {
            const world::hintnode* hintnode =
               world::find_entity(_world.hintnodes,
                                  std::get<world::hintnode_id>(selected));

            if (hintnode) {
               selection_centre += hintnode->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation = hintnode->rotation;
            }
         }
         else if (std::holds_alternative<world::barrier_id>(selected)) {
            const world::barrier* barrier =
               world::find_entity(_world.barriers,
                                  std::get<world::barrier_id>(selected));

            if (barrier) {
               selection_centre += barrier->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation =
                  make_quat_from_euler({0.0f, barrier->rotation_angle, 0.0f});
            }
         }
         else if (std::holds_alternative<world::planning_hub_id>(selected)) {
            const world::planning_hub* planning_hub =
               world::find_entity(_world.planning_hubs,
                                  std::get<world::planning_hub_id>(selected));

            if (planning_hub) {
               selection_centre += planning_hub->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
            }
         }
         else if (std::holds_alternative<world::boundary_id>(selected)) {
            const world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  std::get<world::boundary_id>(selected));

            if (boundary) {
               selection_centre +=
                  {boundary->position.x, 0.0f, boundary->position.y};
               selection_axis_count += {1.0f, 0.0f, 1.0f};
            }
         }
         else if (std::holds_alternative<world::measurement_id>(selected)) {
            const world::measurement* measurement =
               world::find_entity(_world.measurements,
                                  std::get<world::measurement_id>(selected));

            if (measurement) {
               selection_centre += measurement->start;
               selection_centre += measurement->end;
               selection_axis_count += {2.0f, 2.0f, 2.0f};
            }
         }
      }

      selection_axis_count = max(selection_axis_count, float3{1.0f, 1.0f, 1.0f});

      selection_centre /= selection_axis_count;

      const float3 last_move_amount = _move_selection_amount;
      const bool local_space =
         _selection_move_space == selection_transform_space::local;

      if (not local_space or selection_axis_count.x > 1.0f) {
         gizmo_rotation = quaternion{};
      }

      const bool imgui_edited =
         ImGui::DragFloat3("Amount", &_move_selection_amount, 0.05f);
      const bool gizmo_edited = _gizmo.show_translate(selection_centre, gizmo_rotation,
                                                      _move_selection_amount);

      if (imgui_edited or gizmo_edited) {
         const float3 move_delta = (_move_selection_amount - last_move_amount);

         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (std::holds_alternative<world::object_id>(selected)) {
               world::object* object =
                  world::find_entity(_world.objects,
                                     std::get<world::object_id>(selected));

               if (object) {
                  float3 new_position;

                  if (not local_space) {
                     new_position = object->position + move_delta;
                  }
                  else {
                     const quaternion rotation = normalize(object->rotation);

                     new_position =
                        rotation *
                        ((conjugate(rotation) * object->position) + move_delta);
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(&object->position, new_position));
               }
            }
            else if (std::holds_alternative<world::path_id_node_mask>(selected)) {
               const auto& [id, node_mask] =
                  std::get<world::path_id_node_mask>(selected);

               world::path* path = world::find_entity(_world.paths, id);

               if (path) {
                  const std::size_t node_count =
                     std::min(path->nodes.size(), world::max_path_nodes);

                  for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                     if (not node_mask[node_index]) continue;

                     const world::path::node& node = path->nodes[node_index];

                     float3 new_position;

                     if (not local_space) {
                        new_position = node.position + move_delta;
                     }
                     else {
                        const quaternion rotation = normalize(node.rotation);

                        new_position =
                           rotation *
                           ((conjugate(rotation) * node.position) + move_delta);
                     }

                     bundled_edits.push_back(
                        edits::make_set_vector_value(&path->nodes, node_index,
                                                     &world::path::node::position,
                                                     new_position));
                  }
               }
            }
            else if (std::holds_alternative<world::light_id>(selected)) {
               world::light* light =
                  world::find_entity(_world.lights,
                                     std::get<world::light_id>(selected));

               if (light) {
                  float3 new_position;

                  if (not local_space) {
                     new_position = light->position + move_delta;
                  }
                  else {
                     const quaternion rotation = normalize(light->rotation);

                     new_position =
                        rotation *
                        ((conjugate(rotation) * light->position) + move_delta);
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(&light->position, new_position));
               }
            }
            else if (std::holds_alternative<world::region_id>(selected)) {
               world::region* region =
                  world::find_entity(_world.regions,
                                     std::get<world::region_id>(selected));

               if (region) {
                  float3 new_position;

                  if (not local_space) {
                     new_position = region->position + move_delta;
                  }
                  else {
                     const quaternion rotation = normalize(region->rotation);

                     new_position =
                        rotation *
                        ((conjugate(rotation) * region->position) + move_delta);
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(&region->position, new_position));
               }
            }
            else if (std::holds_alternative<world::sector_id>(selected)) {
               world::sector* sector =
                  world::find_entity(_world.sectors,
                                     std::get<world::sector_id>(selected));

               if (sector) {
                  std::vector<float2> new_points = sector->points;

                  for (auto& point : new_points) {
                     point += float2{move_delta.x, move_delta.z};
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(&sector->points, std::move(new_points)));
                  bundled_edits.push_back(
                     edits::make_set_value(&sector->base,
                                           sector->base + move_delta.y));
               }
            }
            else if (std::holds_alternative<world::portal_id>(selected)) {
               world::portal* portal =
                  world::find_entity(_world.portals,
                                     std::get<world::portal_id>(selected));

               if (portal) {
                  float3 new_position;

                  if (not local_space) {
                     new_position = portal->position + move_delta;
                  }
                  else {
                     const quaternion rotation = normalize(portal->rotation);

                     new_position =
                        rotation *
                        ((conjugate(rotation) * portal->position) + move_delta);
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(&portal->position, new_position));
               }
            }
            else if (std::holds_alternative<world::hintnode_id>(selected)) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     std::get<world::hintnode_id>(selected));

               if (hintnode) {
                  float3 new_position;

                  if (not local_space) {
                     new_position = hintnode->position + move_delta;
                  }
                  else {
                     const quaternion rotation = normalize(hintnode->rotation);

                     new_position =
                        rotation *
                        ((conjugate(rotation) * hintnode->position) + move_delta);
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(&hintnode->position, new_position));
               }
            }
            else if (std::holds_alternative<world::barrier_id>(selected)) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     std::get<world::barrier_id>(selected));

               if (barrier) {
                  float3 new_position;

                  if (not local_space) {
                     new_position = barrier->position + move_delta;
                  }
                  else {
                     const quaternion rotation =
                        make_quat_from_euler({0.0f, barrier->rotation_angle, 0.0f});

                     new_position =
                        rotation *
                        ((conjugate(rotation) * barrier->position) + move_delta);
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(&barrier->position, new_position));
               }
            }
            else if (std::holds_alternative<world::planning_hub_id>(selected)) {
               world::planning_hub* planning_hub =
                  world::find_entity(_world.planning_hubs,
                                     std::get<world::planning_hub_id>(selected));

               if (planning_hub) {
                  bundled_edits.push_back(
                     edits::make_set_value(&planning_hub->position,
                                           planning_hub->position + move_delta));
               }
            }
            else if (std::holds_alternative<world::boundary_id>(selected)) {
               world::boundary* boundary =
                  world::find_entity(_world.boundaries,
                                     std::get<world::boundary_id>(selected));

               if (boundary) {
                  bundled_edits.push_back(
                     edits::make_set_value(&boundary->position,
                                           boundary->position + move_delta));
               }
            }
            else if (std::holds_alternative<world::measurement_id>(selected)) {
               world::measurement* measurement =
                  world::find_entity(_world.measurements,
                                     std::get<world::measurement_id>(selected));

               if (measurement) {
                  bundled_edits.push_back(
                     edits::make_set_value(&measurement->start,
                                           measurement->start + move_delta));
                  bundled_edits.push_back(
                     edits::make_set_value(&measurement->end,
                                           measurement->end + move_delta));
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
      }

      if (ImGui::Button("Done", {ImGui::CalcItemWidth(), 0.0f})) {
         open = false;
      }

      ImGui::SeparatorText("Move Space");

      if (ImGui::BeginTable("Move Space", 2,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {
         ImGui::TableNextColumn();
         if (ImGui::Selectable("Local", _selection_move_space ==
                                           selection_transform_space::local)) {
            _selection_move_space = selection_transform_space::local;
            _move_selection_amount = {0.0f, 0.0f, 0.0f};
            _edit_stack_world.close_last();
            _gizmo.deactivate();
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("World", _selection_move_space ==
                                           selection_transform_space::world)) {
            _selection_move_space = selection_transform_space::world;
            _move_selection_amount = {0.0f, 0.0f, 0.0f};
            _edit_stack_world.close_last();
            _gizmo.deactivate();
         }

         ImGui::EndTable();
      }

      ImGui::SeparatorText("Object Gizmo Location");

      if (ImGui::BeginTable("Object Gizmo Location", 2,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {
         ImGui::TableNextColumn();
         if (ImGui::Selectable("Object Position", _gizmo_object_placement ==
                                                     gizmo_object_placement::position)) {
            _gizmo_object_placement = gizmo_object_placement::position;
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Object BBOX Center",
                               _gizmo_object_placement ==
                                  gizmo_object_placement::bbox_centre)) {
            _gizmo_object_placement = gizmo_object_placement::bbox_centre;
         }

         ImGui::EndTable();
      }
   }

   if (not open) {
      _edit_stack_world.close_last();
      _selection_edit_tool = selection_edit_tool::none;
   }

   ImGui::End();
}

}