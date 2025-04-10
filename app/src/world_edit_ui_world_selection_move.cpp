#include "world_edit.hpp"

#include "imgui_ext.hpp"

#include "edits/bundle.hpp"
#include "edits/set_block.hpp"
#include "edits/set_value.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "world/blocks/bounding_box.hpp"
#include "world/blocks/find.hpp"
#include "world/utility/world_utilities.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

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
         if (selected.is<world::object_id>()) {
            const world::object* object =
               world::find_entity(_world.objects, selected.get<world::object_id>());

            if (object) {
               if (_gizmo_object_placement == gizmo_object_placement::position) {
                  selection_centre += object->position;
                  selection_axis_count += {1.0f, 1.0f, 1.0f};
               }
               else {
                  math::bounding_box bbox =
                     _object_classes[object->class_handle].model->bounding_box;

                  bbox = object->rotation * bbox + object->position;

                  selection_centre += ((bbox.max + bbox.min) / 2.0f);
                  selection_axis_count += {1.0f, 1.0f, 1.0f};
               }

               gizmo_rotation = object->rotation;
            }
         }
         else if (selected.is<world::path_id_node_mask>()) {
            const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

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
         else if (selected.is<world::light_id>()) {
            const world::light* light =
               world::find_entity(_world.lights, selected.get<world::light_id>());

            if (light) {
               selection_centre += light->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation = light->rotation;
            }
         }
         else if (selected.is<world::region_id>()) {
            const world::region* region =
               world::find_entity(_world.regions, selected.get<world::region_id>());

            if (region) {
               selection_centre += region->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation = region->rotation;
            }
         }
         else if (selected.is<world::sector_id>()) {
            const world::sector* sector =
               world::find_entity(_world.sectors, selected.get<world::sector_id>());

            if (sector) {
               float3 sector_centre = {};
               float3 sector_axis_count = {};

               for (auto& point : sector->points) {
                  sector_centre += {point.x, 0.0f, point.y};
                  sector_axis_count += {1.0f, 0.0f, 1.0f};
               }

               sector_centre += {0.0f, sector->base + (sector->height / 2.0f), 0.0f};
               sector_axis_count += {0.0f, 1.0f, 0.0f};

               selection_centre += sector_centre;
               selection_axis_count += sector_axis_count;
            }
         }
         else if (selected.is<world::portal_id>()) {
            const world::portal* portal =
               world::find_entity(_world.portals, selected.get<world::portal_id>());

            if (portal) {
               selection_centre += portal->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation = portal->rotation;
            }
         }
         else if (selected.is<world::hintnode_id>()) {
            const world::hintnode* hintnode =
               world::find_entity(_world.hintnodes,
                                  selected.get<world::hintnode_id>());

            if (hintnode) {
               selection_centre += hintnode->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation = hintnode->rotation;
            }
         }
         else if (selected.is<world::barrier_id>()) {
            const world::barrier* barrier =
               world::find_entity(_world.barriers, selected.get<world::barrier_id>());

            if (barrier) {
               selection_centre += barrier->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
               gizmo_rotation =
                  make_quat_from_euler({0.0f, barrier->rotation_angle, 0.0f});
            }
         }
         else if (selected.is<world::planning_hub_id>()) {
            const world::planning_hub* planning_hub =
               world::find_entity(_world.planning_hubs,
                                  selected.get<world::planning_hub_id>());

            if (planning_hub) {
               selection_centre += planning_hub->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
            }
         }
         else if (selected.is<world::boundary_id>()) {
            const world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  selected.get<world::boundary_id>());

            if (boundary) {
               selection_centre += boundary->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
            }
         }
         else if (selected.is<world::measurement_id>()) {
            const world::measurement* measurement =
               world::find_entity(_world.measurements,
                                  selected.get<world::measurement_id>());

            if (measurement) {
               selection_centre += measurement->start;
               selection_centre += measurement->end;
               selection_axis_count += {2.0f, 2.0f, 2.0f};
            }
         }
         else if (selected.is<world::block_id>()) {
            const world::block_id block_id = selected.get<world::block_id>();
            const std::optional<uint32> block_index =
               world::find_block(_world.blocks, block_id);

            if (block_index) {
               const math::bounding_box bbox =
                  world::get_bounding_box(_world.blocks, block_id.type(), *block_index);

               selection_centre += (bbox.min + bbox.max) * 0.5f;
               selection_axis_count += {1.0f, 1.0f, 1.0f};

               switch (block_id.type()) {
               case world::block_type::box: {
                  gizmo_rotation =
                     _world.blocks.boxes.description[*block_index].rotation;
               } break;
               }
            }
         }
      }

      selection_axis_count = max(selection_axis_count, float3{1.0f, 1.0f, 1.0f});

      selection_centre /= selection_axis_count;

      if (_selection_move_space != gizmo_transform_space::local) {
         gizmo_rotation = quaternion{};
      }

      float3 new_selection_centre = selection_centre;

      const bool imgui_edited =
         ImGui::DragFloat3("Position", &new_selection_centre, 0.05f);
      const bool imgui_deactivated = ImGui::IsItemDeactivated();

      const bool gizmo_edited =
         _gizmos.gizmo_position({.name = "Move Selection",
                                 .alignment = _editor_grid_size,
                                 .gizmo_rotation = gizmo_rotation},
                                new_selection_centre);
      const bool gizmo_close_edit = _gizmos.can_close_last_edit();

      if (imgui_edited or gizmo_edited) {
         const float3 move_delta = (new_selection_centre - selection_centre);

         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (selected.is<world::object_id>()) {
               world::object* object =
                  world::find_entity(_world.objects, selected.get<world::object_id>());

               if (object) {
                  bundled_edits.push_back(
                     edits::make_set_value(&object->position,
                                           object->position + move_delta));
               }
            }
            else if (selected.is<world::path_id_node_mask>()) {
               const auto& [id, node_mask] =
                  selected.get<world::path_id_node_mask>();

               world::path* path = world::find_entity(_world.paths, id);

               if (path) {
                  const std::size_t node_count =
                     std::min(path->nodes.size(), world::max_path_nodes);

                  for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                     if (not node_mask[node_index]) continue;

                     const world::path::node& node = path->nodes[node_index];

                     bundled_edits.push_back(
                        edits::make_set_vector_value(&path->nodes, node_index,
                                                     &world::path::node::position,
                                                     node.position + move_delta));
                  }
               }
            }
            else if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (light) {
                  bundled_edits.push_back(
                     edits::make_set_value(&light->position,
                                           light->position + move_delta));
               }
            }
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

               if (region) {
                  bundled_edits.push_back(
                     edits::make_set_value(&region->position,
                                           region->position + move_delta));
               }
            }
            else if (selected.is<world::sector_id>()) {
               world::sector* sector =
                  world::find_entity(_world.sectors, selected.get<world::sector_id>());

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
            else if (selected.is<world::portal_id>()) {
               world::portal* portal =
                  world::find_entity(_world.portals, selected.get<world::portal_id>());

               if (portal) {
                  bundled_edits.push_back(
                     edits::make_set_value(&portal->position,
                                           portal->position + move_delta));
               }
            }
            else if (selected.is<world::hintnode_id>()) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     selected.get<world::hintnode_id>());

               if (hintnode) {
                  bundled_edits.push_back(
                     edits::make_set_value(&hintnode->position,
                                           hintnode->position + move_delta));
               }
            }
            else if (selected.is<world::barrier_id>()) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     selected.get<world::barrier_id>());

               if (barrier) {
                  bundled_edits.push_back(
                     edits::make_set_value(&barrier->position,
                                           barrier->position + move_delta));
               }
            }
            else if (selected.is<world::planning_hub_id>()) {
               world::planning_hub* planning_hub =
                  world::find_entity(_world.planning_hubs,
                                     selected.get<world::planning_hub_id>());

               if (planning_hub) {
                  bundled_edits.push_back(
                     edits::make_set_value(&planning_hub->position,
                                           planning_hub->position + move_delta));
               }
            }
            else if (selected.is<world::boundary_id>()) {
               world::boundary* boundary =
                  world::find_entity(_world.boundaries,
                                     selected.get<world::boundary_id>());

               if (boundary) {
                  bundled_edits.push_back(
                     edits::make_set_value(&boundary->position,
                                           boundary->position + move_delta));
               }
            }
            else if (selected.is<world::measurement_id>()) {
               world::measurement* measurement =
                  world::find_entity(_world.measurements,
                                     selected.get<world::measurement_id>());

               if (measurement) {
                  bundled_edits.push_back(
                     edits::make_set_value(&measurement->start,
                                           measurement->start + move_delta));
                  bundled_edits.push_back(
                     edits::make_set_value(&measurement->end,
                                           measurement->end + move_delta));
               }
            }
            else if (selected.is<world::block_id>()) {
               const world::block_id block_id = selected.get<world::block_id>();
               const std::optional<uint32> block_index =
                  world::find_block(_world.blocks, block_id);

               if (block_index) {
                  switch (block_id.type()) {
                  case world::block_type::box: {
                     const world::block_description_box& box =
                        _world.blocks.boxes.description[*block_index];

                     bundled_edits.push_back(
                        edits::make_set_block_box_metrics(*block_index, box.rotation,
                                                          box.position + move_delta,
                                                          box.size));
                  } break;
                  }
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

      ImGui::SeparatorText("Move Space");

      if (ImGui::BeginTable("Move Space", 2,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {
         ImGui::TableNextColumn();
         if (ImGui::Selectable("Local", _selection_move_space ==
                                           gizmo_transform_space::local)) {
            _selection_move_space = gizmo_transform_space::local;
            _edit_stack_world.close_last();
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("World", _selection_move_space ==
                                           gizmo_transform_space::world)) {
            _selection_move_space = gizmo_transform_space::world;
            _edit_stack_world.close_last();
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