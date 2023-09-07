#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/world_utilities.hpp"

namespace we {

void world_edit::ui_show_world_selection_move() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   if (_interaction_targets.selection.empty()) {
      _selection_edit_tool = selection_edit_tool::none;
   }

   bool open = _selection_edit_tool == selection_edit_tool::move;

   if (ImGui::Begin("Move Selection", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      float3 selection_centre = {0.0f, 0.0f, 0.0f};
      float3 selection_axis_count = {0.0f, 0.0f, 0.0f};

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
            }
         }
         else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
            const auto [id, node_index] = std::get<world::path_id_node_pair>(selected);

            const world::path* path = world::find_entity(_world.paths, id);

            if (path) {
               const world::path::node& node = path->nodes[node_index];

               selection_centre += node.position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
            }
         }
         else if (std::holds_alternative<world::light_id>(selected)) {
            const world::light* light =
               world::find_entity(_world.lights, std::get<world::light_id>(selected));

            if (light) {
               selection_centre += light->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
            }
         }
         else if (std::holds_alternative<world::region_id>(selected)) {
            const world::region* region =
               world::find_entity(_world.regions, std::get<world::region_id>(selected));

            if (region) {
               selection_centre += region->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
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
            }
         }
         else if (std::holds_alternative<world::hintnode_id>(selected)) {
            const world::hintnode* hintnode =
               world::find_entity(_world.hintnodes,
                                  std::get<world::hintnode_id>(selected));

            if (hintnode) {
               selection_centre += hintnode->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
            }
         }
         else if (std::holds_alternative<world::barrier_id>(selected)) {
            const world::barrier* barrier =
               world::find_entity(_world.barriers,
                                  std::get<world::barrier_id>(selected));

            if (barrier) {
               selection_centre += barrier->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
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
      }

      selection_axis_count = max(selection_axis_count, float3{1.0f, 1.0f, 1.0f});

      selection_centre /= selection_axis_count;

      const float3 last_move_amount = _move_selection_amount;

      const bool imgui_edited =
         ImGui::DragFloat3("Amount", &_move_selection_amount, 0.05f);
      const bool gizmo_edited =
         _gizmo.show_translate(selection_centre, _move_selection_amount);

      if (imgui_edited or gizmo_edited) {
         const float3 move_delta = (_move_selection_amount - last_move_amount);

         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (std::holds_alternative<world::object_id>(selected)) {
               const world::object* object =
                  world::find_entity(_world.objects,
                                     std::get<world::object_id>(selected));

               if (object) {
                  bundled_edits.push_back(
                     edits::make_set_value(object->id, &world::object::position,
                                           object->position + move_delta,
                                           object->position));
               }
            }
            else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
               const auto [id, node_index] =
                  std::get<world::path_id_node_pair>(selected);

               const world::path* path = world::find_entity(_world.paths, id);

               if (path) {
                  const world::path::node& node = path->nodes[node_index];

                  bundled_edits.push_back(
                     edits::make_set_path_node_value(path->id, node_index,
                                                     &world::path::node::position,
                                                     node.position + move_delta,
                                                     node.position));
               }
            }
            else if (std::holds_alternative<world::light_id>(selected)) {
               const world::light* light =
                  world::find_entity(_world.lights,
                                     std::get<world::light_id>(selected));

               if (light) {
                  bundled_edits.push_back(
                     edits::make_set_value(light->id, &world::light::position,
                                           light->position + move_delta,
                                           light->position));
               }
            }
            else if (std::holds_alternative<world::region_id>(selected)) {
               const world::region* region =
                  world::find_entity(_world.regions,
                                     std::get<world::region_id>(selected));

               if (region) {
                  bundled_edits.push_back(
                     edits::make_set_value(region->id, &world::region::position,
                                           region->position + move_delta,
                                           region->position));
               }
            }
            else if (std::holds_alternative<world::sector_id>(selected)) {
               const world::sector* sector =
                  world::find_entity(_world.sectors,
                                     std::get<world::sector_id>(selected));

               if (sector) {
                  std::vector<float2> new_points = sector->points;

                  for (auto& point : new_points) {
                     point += float2{move_delta.x, move_delta.z};
                  }

                  bundled_edits.push_back(
                     edits::make_set_value(sector->id, &world::sector::points,
                                           std::move(new_points), sector->points));
               }
            }
            else if (std::holds_alternative<world::portal_id>(selected)) {
               const world::portal* portal =
                  world::find_entity(_world.portals,
                                     std::get<world::portal_id>(selected));

               if (portal) {
                  bundled_edits.push_back(
                     edits::make_set_value(portal->id, &world::portal::position,
                                           portal->position + move_delta,
                                           portal->position));
               }
            }
            else if (std::holds_alternative<world::hintnode_id>(selected)) {
               const world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     std::get<world::hintnode_id>(selected));

               if (hintnode) {
                  bundled_edits.push_back(
                     edits::make_set_value(hintnode->id, &world::hintnode::position,
                                           hintnode->position + move_delta,
                                           hintnode->position));
               }
            }
            else if (std::holds_alternative<world::barrier_id>(selected)) {
               const world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     std::get<world::barrier_id>(selected));

               if (barrier) {
                  bundled_edits.push_back(
                     edits::make_set_value(barrier->id, &world::barrier::position,
                                           barrier->position + move_delta,
                                           barrier->position));
               }
            }
            else if (std::holds_alternative<world::planning_hub_id>(selected)) {
               const world::planning_hub* planning_hub =
                  world::find_entity(_world.planning_hubs,
                                     std::get<world::planning_hub_id>(selected));

               if (planning_hub) {
                  bundled_edits.push_back(
                     edits::make_set_value(planning_hub->id, &world::planning_hub::position,
                                           planning_hub->position + move_delta,
                                           planning_hub->position));
               }
            }
            else if (std::holds_alternative<world::boundary_id>(selected)) {
               const world::boundary* boundary =
                  world::find_entity(_world.boundaries,
                                     std::get<world::boundary_id>(selected));

               if (boundary) {
                  bundled_edits.push_back(
                     edits::make_set_value(boundary->id, &world::boundary::position,
                                           boundary->position +
                                              float2{move_delta.x, move_delta.z},
                                           boundary->position));
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