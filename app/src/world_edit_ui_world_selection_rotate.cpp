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

      for (const auto& selected : _interaction_targets.selection) {
         if (std::holds_alternative<world::object_id>(selected)) {
            const world::object* object =
               world::find_entity(_world.objects, std::get<world::object_id>(selected));

            if (object) {
               selection_centre += object->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
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
      }

      selection_axis_count = max(selection_axis_count, float3{1.0f, 1.0f, 1.0f});

      selection_centre /= selection_axis_count;

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
         _gizmo.show_rotate(selection_centre, _rotate_selection_amount);

      if (imgui_edited or gizmo_edited) {
         const float3 rotate_delta = (_rotate_selection_amount - last_rotation_amount);
         const quaternion rotation = make_quat_from_euler(rotate_delta);

         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (std::holds_alternative<world::object_id>(selected)) {
               const world::object* object =
                  world::find_entity(_world.objects,
                                     std::get<world::object_id>(selected));

               if (object) {
                  bundled_edits.push_back(
                     edits::make_set_value(object->id, &world::object::rotation,
                                           rotation * object->rotation,
                                           object->rotation));
               }
            }
            else if (std::holds_alternative<world::light_id>(selected)) {
               const world::light* light =
                  world::find_entity(_world.lights,
                                     std::get<world::light_id>(selected));

               if (light) {
                  bundled_edits.push_back(
                     edits::make_set_value(light->id, &world::light::rotation,
                                           rotation * light->rotation, light->rotation));
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
                                                     &world::path::node::rotation,
                                                     rotation * node.rotation,
                                                     node.rotation));
               }
            }
            else if (std::holds_alternative<world::region_id>(selected)) {
               const world::region* region =
                  world::find_entity(_world.regions,
                                     std::get<world::region_id>(selected));

               if (region) {
                  bundled_edits.push_back(
                     edits::make_set_value(region->id, &world::region::rotation,
                                           rotation * region->rotation,
                                           region->rotation));
               }
            }
            else if (std::holds_alternative<world::sector_id>(selected)) {
               const world::sector* sector =
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
                     edits::make_set_value(portal->id, &world::portal::rotation,
                                           rotation * portal->rotation,
                                           portal->rotation));
               }
            }
            else if (std::holds_alternative<world::hintnode_id>(selected)) {
               const world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     std::get<world::hintnode_id>(selected));

               if (hintnode) {
                  bundled_edits.push_back(
                     edits::make_set_value(hintnode->id, &world::hintnode::rotation,
                                           rotation * hintnode->rotation,
                                           hintnode->rotation));
               }
            }
            else if (std::holds_alternative<world::barrier_id>(selected)) {
               const world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     std::get<world::barrier_id>(selected));

               if (barrier) {
                  bundled_edits.push_back(
                     edits::make_set_value(barrier->id, &world::barrier::rotation_angle,
                                           barrier->rotation_angle +
                                              rotate_delta.y,
                                           barrier->rotation_angle));
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
