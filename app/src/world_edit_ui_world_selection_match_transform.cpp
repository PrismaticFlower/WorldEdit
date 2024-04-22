#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/world_utilities.hpp"

namespace we {

void world_edit::ui_show_world_selection_match_transform() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   bool open = _selection_edit_tool == selection_edit_tool::match_transform;

   if (ImGui::Begin("Match Transform", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Checkbox("Match Rotation",
                      &_selection_match_transform_config.match_rotation);

      ImGui::SetItemTooltip(
         "Only Objects, Lights, Path Nodes, Regions, Portals and Hintnodes can "
         "have their rotation matched.");

      ImGui::Checkbox("Match Position",
                      &_selection_match_transform_config.match_position);

      ImGui::Separator();

      ImGui::TextWrapped(
         "Pick an entity with the cursor. All the entities in the selection "
         "will have their transforms set to match the picked entity.");

      if (std::exchange(_selection_match_transform_context.clicked, false) and
          _interaction_targets.hovered_entity and
          not std::holds_alternative<world::planning_connection_id>(
             *_interaction_targets.hovered_entity)) {
         float3 new_position = {0.0f, 0.0f, 0.0f};
         quaternion new_rotation = {1.0f, 0.0f, 0.0f, 0.0f};

         edits::bundle_vector bundled_edits;

         bool hovered_has_rotation = false;

         if (const world::hovered_entity& hovered = *_interaction_targets.hovered_entity;
             std::holds_alternative<world::object_id>(hovered)) {
            world::object* object =
               world::find_entity(_world.objects, std::get<world::object_id>(hovered));

            if (object) {
               new_position = object->position;
               new_rotation = object->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (std::holds_alternative<world::light_id>(hovered)) {
            world::light* light =
               world::find_entity(_world.lights, std::get<world::light_id>(hovered));

            if (light) {
               new_position = light->position;
               new_rotation = light->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (std::holds_alternative<world::path_id_node_mask>(hovered)) {
            const auto& [id, node_mask] = std::get<world::path_id_node_mask>(hovered);

            world::path* path = world::find_entity(_world.paths, id);

            if (path) {
               const std::size_t node_count =
                  std::min(path->nodes.size(), world::max_path_nodes);

               for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                  if (not node_mask[node_index]) continue;

                  new_position = path->nodes[node_index].position;
                  new_rotation = path->nodes[node_index].rotation;
                  hovered_has_rotation = true;

                  break;
               }
            }
         }
         else if (std::holds_alternative<world::region_id>(hovered)) {
            world::region* region =
               world::find_entity(_world.regions, std::get<world::region_id>(hovered));

            if (region) {
               new_position = region->position;
               new_rotation = region->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (std::holds_alternative<world::sector_id>(hovered)) {
            world::sector* sector =
               world::find_entity(_world.sectors, std::get<world::sector_id>(hovered));

            if (sector) {
               float2 sector_centre = {0.0f, 0.0f};

               for (const float2& point : sector->points) {
                  sector_centre += point;
               }

               sector_centre /= static_cast<float>(sector->points.size());

               new_position = {sector_centre.x, sector->base, sector_centre.y};
               hovered_has_rotation = false;
            }
         }
         else if (std::holds_alternative<world::portal_id>(hovered)) {
            world::portal* portal =
               world::find_entity(_world.portals, std::get<world::portal_id>(hovered));

            if (portal) {
               new_position = portal->position;
               new_rotation = portal->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (std::holds_alternative<world::hintnode_id>(hovered)) {
            world::hintnode* hintnode =
               world::find_entity(_world.hintnodes,
                                  std::get<world::hintnode_id>(hovered));

            if (hintnode) {
               new_position = hintnode->position;
               new_rotation = hintnode->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (std::holds_alternative<world::barrier_id>(hovered)) {
            world::barrier* barrier =
               world::find_entity(_world.barriers,
                                  std::get<world::barrier_id>(hovered));

            if (barrier) {
               new_position = barrier->position;
               hovered_has_rotation = false;
            }
         }
         else if (std::holds_alternative<world::planning_hub_id>(hovered)) {
            world::planning_hub* hub =
               world::find_entity(_world.planning_hubs,
                                  std::get<world::planning_hub_id>(hovered));

            if (hub) {
               new_position = hub->position;
               hovered_has_rotation = false;
            }
         }
         else if (std::holds_alternative<world::boundary_id>(hovered)) {
            world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  std::get<world::boundary_id>(hovered));

            if (boundary) {
               new_position = boundary->position;
               hovered_has_rotation = false;
            }
         }
         else if (std::holds_alternative<world::measurement_id>(hovered)) {
            world::measurement* measurement =
               world::find_entity(_world.measurements,
                                  std::get<world::measurement_id>(hovered));

            if (measurement) {
               new_position = (measurement->start + measurement->end) * 0.5f;
               hovered_has_rotation = false;
            }
         }

         const bool match_rotation =
            _selection_match_transform_config.match_rotation and hovered_has_rotation;
         const bool match_position = _selection_match_transform_config.match_position;

         for (const auto& selected : _interaction_targets.selection) {
            if (std::holds_alternative<world::object_id>(selected)) {
               world::object* object =
                  world::find_entity(_world.objects,
                                     std::get<world::object_id>(selected));

               if (object) {
                  if (match_rotation) {
                     bundled_edits.push_back(
                        edits::make_set_value(&object->rotation, new_rotation));
                  }

                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&object->position, new_position));
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

                     if (match_rotation) {
                        bundled_edits.push_back(
                           edits::make_set_vector_value(&path->nodes, node_index,
                                                        &world::path::node::rotation,
                                                        new_rotation));
                     }

                     if (match_position) {
                        bundled_edits.push_back(
                           edits::make_set_vector_value(&path->nodes, node_index,
                                                        &world::path::node::position,
                                                        new_position));
                     }
                  }
               }
            }
            else if (std::holds_alternative<world::light_id>(selected)) {
               world::light* light =
                  world::find_entity(_world.lights,
                                     std::get<world::light_id>(selected));

               if (light) {
                  if (match_rotation) {
                     bundled_edits.push_back(
                        edits::make_set_value(&light->rotation, new_rotation));
                  }

                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&light->position, new_position));
                  }
               }
            }
            else if (std::holds_alternative<world::region_id>(selected)) {
               world::region* region =
                  world::find_entity(_world.regions,
                                     std::get<world::region_id>(selected));

               if (region) {
                  if (match_rotation) {
                     bundled_edits.push_back(
                        edits::make_set_value(&region->rotation, new_rotation));
                  }

                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&region->position, new_position));
                  }
               }
            }
            else if (std::holds_alternative<world::sector_id>(selected)) {
               world::sector* sector =
                  world::find_entity(_world.sectors,
                                     std::get<world::sector_id>(selected));

               if (sector) {
                  if (match_position) {
                     std::vector<float2> new_points = sector->points;

                     float2 sector_centre = {0.0f, 0.0f};

                     for (float2& point : new_points) sector_centre += point;

                     sector_centre /= static_cast<float>(new_points.size());

                     for (float2& point : new_points) {
                        point = point - sector_centre +
                                float2{new_position.x, new_position.z};
                     }

                     bundled_edits.push_back(
                        edits::make_set_value(&sector->points, std::move(new_points)));
                     bundled_edits.push_back(
                        edits::make_set_value(&sector->base, new_position.y));
                  }
               }
            }
            else if (std::holds_alternative<world::portal_id>(selected)) {
               world::portal* portal =
                  world::find_entity(_world.portals,
                                     std::get<world::portal_id>(selected));

               if (portal) {
                  if (match_rotation) {
                     bundled_edits.push_back(
                        edits::make_set_value(&portal->rotation, new_rotation));
                  }

                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&portal->position, new_position));
                  }
               }
            }
            else if (std::holds_alternative<world::hintnode_id>(selected)) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     std::get<world::hintnode_id>(selected));

               if (hintnode) {
                  if (match_rotation) {
                     bundled_edits.push_back(
                        edits::make_set_value(&hintnode->rotation, new_rotation));
                  }

                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&hintnode->position, new_position));
                  }
               }
            }
            else if (std::holds_alternative<world::barrier_id>(selected)) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     std::get<world::barrier_id>(selected));

               if (barrier) {
                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&barrier->position, new_position));
                  }
               }
            }
            else if (std::holds_alternative<world::planning_hub_id>(selected)) {
               world::planning_hub* planning_hub =
                  world::find_entity(_world.planning_hubs,
                                     std::get<world::planning_hub_id>(selected));

               if (planning_hub) {
                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&planning_hub->position, new_position));
                  }
               }
            }
            else if (std::holds_alternative<world::boundary_id>(selected)) {
               world::boundary* boundary =
                  world::find_entity(_world.boundaries,
                                     std::get<world::boundary_id>(selected));

               if (boundary) {
                  bundled_edits.push_back(
                     edits::make_set_value(&boundary->position, new_position));
               }
            }
            else if (std::holds_alternative<world::measurement_id>(selected)) {
               world::measurement* measurement =
                  world::find_entity(_world.measurements,
                                     std::get<world::measurement_id>(selected));

               if (measurement) {
                  if (match_position) {
                     const float3 old_centre =
                        (measurement->start + measurement->end) * 0.5f;

                     bundled_edits.push_back(
                        edits::make_set_multi_value(&measurement->start,
                                                    measurement->start - old_centre + new_position,
                                                    &measurement->end,
                                                    measurement->end - old_centre +
                                                       new_position));
                  }
               }
            }
         }

         if (bundled_edits.size() == 1) {
            _edit_stack_world.apply(std::move(bundled_edits.back()),
                                    _edit_context, {.closed = true});
         }
         else if (not bundled_edits.empty()) {
            _edit_stack_world.apply(edits::make_bundle(std::move(bundled_edits)),
                                    _edit_context, {.closed = true});
         }

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