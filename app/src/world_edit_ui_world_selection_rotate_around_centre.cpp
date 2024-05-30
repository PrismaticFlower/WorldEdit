#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <numbers>

namespace we {

void world_edit::ui_show_world_selection_rotate_around_centre() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = _selection_edit_tool == selection_edit_tool::rotate_around_centre;

   if (ImGui::Begin("Rotate Selection##centred", &open,
                    ImGuiWindowFlags_AlwaysAutoResize)) {

      const float3 last_rotation_amount = _rotate_selection_amount;

      float3 rotate_selection_amount_degrees =
         _rotate_selection_amount * 180.0f / std::numbers::pi_v<float>;

      const bool imgui_edited =
         ImGui::DragFloat3("Amount", &rotate_selection_amount_degrees, 1.0f);

      if (imgui_edited) {
         _rotate_selection_amount =
            rotate_selection_amount_degrees * std::numbers::pi_v<float> / 180.0f;
      }

      const bool gizmo_edited =
         _gizmo.show_rotate(_rotate_selection_centre, quaternion{},
                            _rotate_selection_amount);

      if (imgui_edited or gizmo_edited) {
         const float3 rotate_delta = (_rotate_selection_amount - last_rotation_amount);
         const quaternion rotation = make_quat_from_euler(rotate_delta);
         const float3 centre = _rotate_selection_centre;

         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (std::holds_alternative<world::object_id>(selected)) {
               world::object* object =
                  world::find_entity(_world.objects,
                                     std::get<world::object_id>(selected));

               if (object) {
                  bundled_edits.push_back(
                     edits::make_set_value(&object->rotation,
                                           rotation * object->rotation));
                  bundled_edits.push_back(
                     edits::make_set_value(&object->position,
                                           (rotation * (object->position - centre)) +
                                              centre));
               }
            }
            else if (std::holds_alternative<world::light_id>(selected)) {
               world::light* light =
                  world::find_entity(_world.lights,
                                     std::get<world::light_id>(selected));

               if (light) {
                  bundled_edits.push_back(
                     edits::make_set_value(&light->rotation, rotation * light->rotation));
                  bundled_edits.push_back(
                     edits::make_set_value(&light->position,
                                           (rotation * (light->position - centre)) +
                                              centre));

                  if (world::is_region_light(*light)) {
                     bundled_edits.push_back(
                        edits::make_set_value(&light->region_rotation,
                                              rotation * light->region_rotation));
                  }
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
                                                     rotation * node.rotation));
                     bundled_edits.push_back(edits::make_set_vector_value(
                        &path->nodes, node_index, &world::path::node::position,
                        (rotation * (node.position - centre)) + centre));
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
                                           rotation * region->rotation));
                  bundled_edits.push_back(
                     edits::make_set_value(&region->position,
                                           (rotation * (region->position - centre)) +
                                              centre));
               }
            }
            else if (std::holds_alternative<world::sector_id>(selected)) {
               world::sector* sector =
                  world::find_entity(_world.sectors,
                                     std::get<world::sector_id>(selected));

               if (sector) {
                  const quaternion sector_rotation =
                     make_quat_from_euler(float3{0.0f, rotate_delta.y, 0.0f});

                  float2 sector_centre = {0.0f, 0.0f};

                  for (const auto& point : sector->points) {
                     sector_centre += (point);
                  }

                  sector_centre /= static_cast<float>(sector->points.size());

                  const float3 rotated_sector_centre =
                     (rotation *
                      (float3{sector_centre.x, 0.0f, sector_centre.y} - centre)) +
                     centre;

                  std::vector<float2> new_points = sector->points;

                  for (auto& point : new_points) {
                     const float3 rotated_point =
                        sector_rotation * float3{point.x - sector_centre.x, 0.0f,
                                                 point.y - sector_centre.y};

                     point =
                        float2{rotated_point.x, rotated_point.z} +
                        float2{rotated_sector_centre.x, rotated_sector_centre.z};
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
                                           rotation * portal->rotation));
                  bundled_edits.push_back(
                     edits::make_set_value(&portal->position,
                                           (rotation * (portal->position - centre)) +
                                              centre));
               }
            }
            else if (std::holds_alternative<world::hintnode_id>(selected)) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     std::get<world::hintnode_id>(selected));

               if (hintnode) {
                  bundled_edits.push_back(
                     edits::make_set_value(&hintnode->rotation,
                                           rotation * hintnode->rotation));
                  bundled_edits.push_back(
                     edits::make_set_value(&hintnode->position,
                                           (rotation * (hintnode->position - centre)) +
                                              centre));
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
                  bundled_edits.push_back(
                     edits::make_set_value(&barrier->position,
                                           (rotation * (barrier->position - centre)) +
                                              centre));
               }
            }
            else if (std::holds_alternative<world::planning_hub_id>(selected)) {
               world::planning_hub* hub =
                  world::find_entity(_world.planning_hubs,
                                     std::get<world::planning_hub_id>(selected));

               if (hub) {
                  bundled_edits.push_back(
                     edits::make_set_value(&hub->position,
                                           (rotation * (hub->position - centre)) + centre));
               }
            }
            else if (std::holds_alternative<world::boundary_id>(selected)) {
               world::boundary* boundary =
                  world::find_entity(_world.boundaries,
                                     std::get<world::boundary_id>(selected));

               if (boundary) {
                  bundled_edits.push_back(
                     edits::make_set_value(&boundary->position,
                                           (rotation * (boundary->position - centre)) +
                                              centre));
               }
            }
            else if (std::holds_alternative<world::measurement_id>(selected)) {
               world::measurement* measurement =
                  world::find_entity(_world.measurements,
                                     std::get<world::measurement_id>(selected));

               if (measurement) {
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

      if (ImGui::Button("Done", {ImGui::CalcItemWidth(), 0.0f})) {
         open = false;
      }
   }

   if (not open) {
      _edit_stack_world.close_last();
      _selection_edit_tool = selection_edit_tool::none;
   }

   ImGui::End();
}

}