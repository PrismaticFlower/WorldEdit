#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/world_utilities.hpp"

#include <numbers>

namespace we {

void world_edit::ui_show_world_selection_rotate() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = _selection_edit_tool == selection_edit_tool::rotate;

   if (ImGui::Begin("Rotate Selection", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      float3 selection_centre = {0.0f, 0.0f, 0.0f};
      float3 selection_axis_count = {0.0f, 0.0f, 0.0f};
      quaternion gizmo_rotation;

      for (const auto& selected : _interaction_targets.selection) {
         if (std::holds_alternative<world::object_id>(selected)) {
            const world::object* object =
               world::find_entity(_world.objects, std::get<world::object_id>(selected));

            if (object) {
               selection_centre += object->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
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

      const float3 last_rotation_amount = _rotate_selection_amount;
      const bool local_space =
         _selection_rotate_space == selection_transform_space::local;

      float3 rotate_selection_amount_degrees =
         _rotate_selection_amount * 180.0f / std::numbers::pi_v<float>;

      const bool imgui_edited =
         ImGui::DragFloat3("Amount", &rotate_selection_amount_degrees, 1.0f);
      const bool imgui_deactivated = ImGui::IsItemDeactivated();

      if (imgui_edited) {
         _rotate_selection_amount =
            rotate_selection_amount_degrees * std::numbers::pi_v<float> / 180.0f;
      }

      if (not local_space or selection_axis_count.x > 1.0f) {
         gizmo_rotation = quaternion{};
      }

      const bool gizmo_edited = _gizmo.show_rotate(selection_centre, gizmo_rotation,
                                                   _rotate_selection_amount);
      const bool gizmo_close_edit = _gizmo.can_close_last_edit();

      if (imgui_edited or gizmo_edited) {
         const float3 rotate_delta = (_rotate_selection_amount - last_rotation_amount);
         const quaternion rotation = make_quat_from_euler(rotate_delta);

         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (std::holds_alternative<world::object_id>(selected)) {
               world::object* object =
                  world::find_entity(_world.objects,
                                     std::get<world::object_id>(selected));

               if (object) {
                  bundled_edits.push_back(
                     edits::make_set_value(&object->rotation,
                                           local_space ? object->rotation * rotation
                                                       : rotation * object->rotation));
               }
            }
            else if (std::holds_alternative<world::light_id>(selected)) {
               world::light* light =
                  world::find_entity(_world.lights,
                                     std::get<world::light_id>(selected));

               if (light) {
                  bundled_edits.push_back(
                     edits::make_set_value(&light->rotation,
                                           local_space ? light->rotation * rotation
                                                       : rotation * light->rotation));
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

                     bundled_edits.push_back(
                        edits::make_set_vector_value(&path->nodes, node_index,
                                                     &world::path::node::rotation,
                                                     local_space
                                                        ? node.rotation * rotation
                                                        : rotation * node.rotation));
                  }
               }
            }
            else if (std::holds_alternative<world::region_id>(selected)) {
               world::region* region =
                  world::find_entity(_world.regions,
                                     std::get<world::region_id>(selected));

               if (region) {
                  bundled_edits.push_back(
                     edits::make_set_value(&region->rotation,
                                           local_space ? region->rotation * rotation
                                                       : rotation * region->rotation));
               }
            }
            else if (std::holds_alternative<world::sector_id>(selected)) {
               world::sector* sector =
                  world::find_entity(_world.sectors,
                                     std::get<world::sector_id>(selected));

               if (sector) {
                  const quaternion sector_rotation =
                     make_quat_from_euler(float3{0.0f, rotate_delta.y, 0.0f});

                  float2 centre = {0.0f, 0.0f};

                  for (const auto& point : sector->points) {
                     centre += point;
                  }

                  centre /= static_cast<float>(sector->points.size());

                  std::vector<float2> new_points = sector->points;

                  for (auto& point : new_points) {
                     const float3 rotated_point =
                        sector_rotation *
                        float3{point.x - centre.x, 0.0f, point.y - centre.y};

                     point = float2{rotated_point.x, rotated_point.z} + centre;
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(&sector->points, std::move(new_points)));
               }
            }
            else if (std::holds_alternative<world::portal_id>(selected)) {
               world::portal* portal =
                  world::find_entity(_world.portals,
                                     std::get<world::portal_id>(selected));

               if (portal) {
                  bundled_edits.push_back(
                     edits::make_set_value(&portal->rotation,
                                           local_space ? portal->rotation * rotation
                                                       : rotation * portal->rotation));
               }
            }
            else if (std::holds_alternative<world::hintnode_id>(selected)) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     std::get<world::hintnode_id>(selected));

               if (hintnode) {
                  bundled_edits.push_back(
                     edits::make_set_value(&hintnode->rotation,
                                           local_space
                                              ? hintnode->rotation * rotation
                                              : rotation * hintnode->rotation));
               }
            }
            else if (std::holds_alternative<world::barrier_id>(selected)) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     std::get<world::barrier_id>(selected));

               if (barrier) {
                  bundled_edits.push_back(
                     edits::make_set_value(&barrier->rotation_angle,
                                           barrier->rotation_angle +
                                              rotate_delta.y));
               }
            }
            else if (std::holds_alternative<world::measurement_id>(selected)) {
               world::measurement* measurement =
                  world::find_entity(_world.measurements,
                                     std::get<world::measurement_id>(selected));

               if (measurement) {
                  const float3 centre = (measurement->start + measurement->end) * 0.5f;

                  bundled_edits.push_back(
                     edits::make_set_value(&measurement->start,
                                           (rotation * (measurement->start - centre)) +
                                              centre));
                  bundled_edits.push_back(
                     edits::make_set_value(&measurement->end,
                                           (rotation * (measurement->end - centre)) +
                                              centre));
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

      if (imgui_deactivated or gizmo_close_edit) {
         _edit_stack_world.close_last();
      }

      if (ImGui::Button("Done", {ImGui::CalcItemWidth(), 0.0f})) {
         open = false;
      }

      ImGui::SeparatorText("Rotate Space");

      if (ImGui::BeginTable("Rotate Space", 2,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {
         ImGui::TableNextColumn();
         if (ImGui::Selectable("Local", _selection_rotate_space ==
                                           selection_transform_space::local)) {
            _selection_rotate_space = selection_transform_space::local;
            _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
            _edit_stack_world.close_last();
            _gizmo.deactivate();
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("World", _selection_rotate_space ==
                                           selection_transform_space::world)) {
            _selection_rotate_space = selection_transform_space::world;
            _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
            _edit_stack_world.close_last();
            _gizmo.deactivate();
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
