#include "world_edit.hpp"

#include "imgui_ext.hpp"

#include "edits/bundle.hpp"
#include "edits/set_block.hpp"
#include "edits/set_value.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "world/blocks/utility/bounding_box.hpp"
#include "world/blocks/utility/find.hpp"
#include "world/utility/world_utilities.hpp"

#include <numbers>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

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
         if (selected.is<world::object_id>()) {
            const world::object* object =
               world::find_entity(_world.objects, selected.get<world::object_id>());

            if (object) {
               selection_centre += object->position;
               selection_axis_count += {1.0f, 1.0f, 1.0f};
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
               for (auto& point : sector->points) {
                  selection_centre += {point.x, 0.0f, point.y};
                  selection_axis_count += {1.0f, 0.0f, 1.0f};
               }
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
               case world::block_type::ramp: {
                  gizmo_rotation =
                     _world.blocks.ramps.description[*block_index].rotation;
               } break;
               case world::block_type::quad: {
               } break;
               case world::block_type::cylinder: {
                  gizmo_rotation =
                     _world.blocks.cylinders.description[*block_index].rotation;
               } break;
               case world::block_type::custom: {
                  gizmo_rotation =
                     _world.blocks.custom.description[*block_index].rotation;
               } break;
               case world::block_type::cone: {
                  gizmo_rotation =
                     _world.blocks.cones.description[*block_index].rotation;
               } break;
               case world::block_type::hemisphere: {
                  gizmo_rotation =
                     _world.blocks.hemispheres.description[*block_index].rotation;
               } break;
               case world::block_type::pyramid: {
                  gizmo_rotation =
                     _world.blocks.pyramids.description[*block_index].rotation;
               } break;
               }
            }
         }
      }

      selection_axis_count = max(selection_axis_count, float3{1.0f, 1.0f, 1.0f});

      selection_centre /= selection_axis_count;

      const float3 last_rotation_amount = _rotate_selection_amount;
      const bool local_space = _selection_rotate_space == gizmo_transform_space::local;

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

      const bool gizmo_edited =
         _gizmos.gizmo_rotation({.name = "Rotate Selection",
                                 .gizmo_rotation = gizmo_rotation,
                                 .gizmo_positionWS = selection_centre},
                                _rotate_selection_amount);
      const bool gizmo_close_edit = _gizmos.can_close_last_edit();

      if (imgui_edited or gizmo_edited) {
         const float3 rotate_delta = (_rotate_selection_amount - last_rotation_amount);
         const quaternion rotation = make_quat_from_euler(rotate_delta);

         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (selected.is<world::object_id>()) {
               world::object* object =
                  world::find_entity(_world.objects, selected.get<world::object_id>());

               if (object) {
                  bundled_edits.push_back(
                     edits::make_set_value(&object->rotation,
                                           local_space ? object->rotation * rotation
                                                       : rotation * object->rotation));
               }
            }
            else if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (light) {
                  bundled_edits.push_back(
                     edits::make_set_value(&light->rotation,
                                           local_space ? light->rotation * rotation
                                                       : rotation * light->rotation));
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
                                                     &world::path::node::rotation,
                                                     local_space
                                                        ? node.rotation * rotation
                                                        : rotation * node.rotation));
                  }
               }
            }
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

               if (region) {
                  bundled_edits.push_back(
                     edits::make_set_value(&region->rotation,
                                           local_space ? region->rotation * rotation
                                                       : rotation * region->rotation));
               }
            }
            else if (selected.is<world::sector_id>()) {
               world::sector* sector =
                  world::find_entity(_world.sectors, selected.get<world::sector_id>());

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
            else if (selected.is<world::portal_id>()) {
               world::portal* portal =
                  world::find_entity(_world.portals, selected.get<world::portal_id>());

               if (portal) {
                  bundled_edits.push_back(
                     edits::make_set_value(&portal->rotation,
                                           local_space ? portal->rotation * rotation
                                                       : rotation * portal->rotation));
               }
            }
            else if (selected.is<world::hintnode_id>()) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     selected.get<world::hintnode_id>());

               if (hintnode) {
                  bundled_edits.push_back(
                     edits::make_set_value(&hintnode->rotation,
                                           local_space
                                              ? hintnode->rotation * rotation
                                              : rotation * hintnode->rotation));
               }
            }
            else if (selected.is<world::barrier_id>()) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     selected.get<world::barrier_id>());

               if (barrier) {
                  bundled_edits.push_back(
                     edits::make_set_value(&barrier->rotation_angle,
                                           barrier->rotation_angle +
                                              rotate_delta.y));
               }
            }
            else if (selected.is<world::measurement_id>()) {
               world::measurement* measurement =
                  world::find_entity(_world.measurements,
                                     selected.get<world::measurement_id>());

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
                        edits::make_set_block_box_metrics(*block_index,
                                                          local_space
                                                             ? box.rotation * rotation
                                                             : rotation * box.rotation,
                                                          box.position, box.size));
                  } break;
                  case world::block_type::ramp: {
                     const world::block_description_ramp& ramp =
                        _world.blocks.ramps.description[*block_index];

                     bundled_edits.push_back(
                        edits::make_set_block_ramp_metrics(*block_index,
                                                           local_space
                                                              ? ramp.rotation * rotation
                                                              : rotation * ramp.rotation,
                                                           ramp.position, ramp.size));
                  } break;
                  case world::block_type::quad: {
                     const world::block_description_quad& quad =
                        _world.blocks.quads.description[*block_index];

                     const float3 centre = (quad.vertices[0] + quad.vertices[1] +
                                            quad.vertices[2] + quad.vertices[3]) /
                                           4.0f;

                     bundled_edits.push_back(edits::make_set_block_quad_metrics(
                        *block_index,
                        {(rotation * (quad.vertices[0] - centre)) + centre,
                         (rotation * (quad.vertices[1] - centre)) + centre,
                         (rotation * (quad.vertices[2] - centre)) + centre,
                         (rotation * (quad.vertices[3] - centre)) + centre}));
                  } break;
                  case world::block_type::cylinder: {
                     const world::block_description_cylinder& cylinder =
                        _world.blocks.cylinders.description[*block_index];

                     bundled_edits.push_back(edits::make_set_block_cylinder_metrics(
                        *block_index,
                        local_space ? cylinder.rotation * rotation
                                    : rotation * cylinder.rotation,
                        cylinder.position, cylinder.size));
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     bundled_edits.push_back(edits::make_set_block_custom_metrics(
                        *block_index,
                        local_space ? block.rotation * rotation
                                    : rotation * block.rotation,
                        block.position, block.mesh_description));
                  } break;
                  case world::block_type::cone: {
                     const world::block_description_cone& cone =
                        _world.blocks.cones.description[*block_index];

                     bundled_edits.push_back(
                        edits::make_set_block_cone_metrics(*block_index,
                                                           local_space
                                                              ? cone.rotation * rotation
                                                              : rotation * cone.rotation,
                                                           cone.position, cone.size));
                  } break;
                  case world::block_type::hemisphere: {
                     const world::block_description_hemisphere& hemisphere =
                        _world.blocks.hemispheres.description[*block_index];

                     bundled_edits.push_back(edits::make_set_block_hemisphere_metrics(
                        *block_index,
                        local_space ? hemisphere.rotation * rotation
                                    : rotation * hemisphere.rotation,
                        hemisphere.position, hemisphere.size));
                  } break;
                  case world::block_type::pyramid: {
                     const world::block_description_pyramid& pyramid =
                        _world.blocks.pyramids.description[*block_index];

                     bundled_edits.push_back(edits::make_set_block_pyramid_metrics(
                        *block_index,
                        local_space ? pyramid.rotation * rotation
                                    : rotation * pyramid.rotation,
                        pyramid.position, pyramid.size));
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

      ImGui::SeparatorText("Rotate Space");

      if (ImGui::BeginTable("Rotate Space", 2,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {
         ImGui::TableNextColumn();
         if (ImGui::Selectable("Local", _selection_rotate_space ==
                                           gizmo_transform_space::local)) {
            _selection_rotate_space = gizmo_transform_space::local;
            _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
            _edit_stack_world.close_last();
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("World", _selection_rotate_space ==
                                           gizmo_transform_space::world)) {
            _selection_rotate_space = gizmo_transform_space::world;
            _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
            _edit_stack_world.close_last();
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
