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

      ImGui::SetItemTooltip("Only Objects, Lights, Path Nodes, Regions, "
                            "Portals, Hintnodes and some Block types can "
                            "have their rotation matched.");

      ImGui::Checkbox("Match Position",
                      &_selection_match_transform_config.match_position);

      ImGui::Separator();

      ImGui::TextWrapped(
         "Pick an entity with the cursor. All the entities in the selection "
         "will have their transforms set to match the picked entity.");

      if (std::exchange(_selection_match_transform_context.clicked, false) and
          _interaction_targets.hovered_entity and
          not _interaction_targets.hovered_entity->is<world::planning_connection_id>()) {
         float3 new_position = {0.0f, 0.0f, 0.0f};
         quaternion new_rotation = {1.0f, 0.0f, 0.0f, 0.0f};

         edits::bundle_vector bundled_edits;

         bool hovered_has_rotation = false;

         if (const world::hovered_entity& hovered = *_interaction_targets.hovered_entity;
             hovered.is<world::object_id>()) {
            world::object* object =
               world::find_entity(_world.objects, hovered.get<world::object_id>());

            if (object) {
               new_position = object->position;
               new_rotation = object->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (hovered.is<world::light_id>()) {
            world::light* light =
               world::find_entity(_world.lights, hovered.get<world::light_id>());

            if (light) {
               new_position = light->position;
               new_rotation = light->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (hovered.is<world::path_id_node_mask>()) {
            const auto& [id, node_mask] = hovered.get<world::path_id_node_mask>();

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
         else if (hovered.is<world::region_id>()) {
            world::region* region =
               world::find_entity(_world.regions, hovered.get<world::region_id>());

            if (region) {
               new_position = region->position;
               new_rotation = region->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (hovered.is<world::sector_id>()) {
            world::sector* sector =
               world::find_entity(_world.sectors, hovered.get<world::sector_id>());

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
         else if (hovered.is<world::portal_id>()) {
            world::portal* portal =
               world::find_entity(_world.portals, hovered.get<world::portal_id>());

            if (portal) {
               new_position = portal->position;
               new_rotation = portal->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (hovered.is<world::hintnode_id>()) {
            world::hintnode* hintnode =
               world::find_entity(_world.hintnodes, hovered.get<world::hintnode_id>());

            if (hintnode) {
               new_position = hintnode->position;
               new_rotation = hintnode->rotation;
               hovered_has_rotation = true;
            }
         }
         else if (hovered.is<world::barrier_id>()) {
            world::barrier* barrier =
               world::find_entity(_world.barriers, hovered.get<world::barrier_id>());

            if (barrier) {
               new_position = barrier->position;
               hovered_has_rotation = false;
            }
         }
         else if (hovered.is<world::planning_hub_id>()) {
            world::planning_hub* hub =
               world::find_entity(_world.planning_hubs,
                                  hovered.get<world::planning_hub_id>());

            if (hub) {
               new_position = hub->position;
               hovered_has_rotation = false;
            }
         }
         else if (hovered.is<world::boundary_id>()) {
            world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  hovered.get<world::boundary_id>());

            if (boundary) {
               new_position = boundary->position;
               hovered_has_rotation = false;
            }
         }
         else if (hovered.is<world::measurement_id>()) {
            world::measurement* measurement =
               world::find_entity(_world.measurements,
                                  hovered.get<world::measurement_id>());

            if (measurement) {
               new_position = (measurement->start + measurement->end) * 0.5f;
               hovered_has_rotation = false;
            }
         }
         else if (hovered.is<world::block_id>()) {
            const world::block_id block_id = hovered.get<world::block_id>();
            const std::optional<uint32> block_index =
               world::find_block(_world.blocks, block_id);

            if (block_index) {
               const math::bounding_box bbox =
                  world::get_bounding_box(_world.blocks, block_id.type(), *block_index);

               new_position = (bbox.min + bbox.max) * 0.5f;

               switch (block_id.type()) {
               case world::block_type::box: {
                  new_rotation = _world.blocks.boxes.description[*block_index].rotation;
               } break;
               }
            }
         }

         const bool match_rotation =
            _selection_match_transform_config.match_rotation and hovered_has_rotation;
         const bool match_position = _selection_match_transform_config.match_position;

         for (const auto& selected : _interaction_targets.selection) {
            if (selected.is<world::object_id>()) {
               world::object* object =
                  world::find_entity(_world.objects, selected.get<world::object_id>());

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
            else if (selected.is<world::path_id_node_mask>()) {
               const auto& [id, node_mask] =
                  selected.get<world::path_id_node_mask>();

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
            else if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

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
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

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
            else if (selected.is<world::sector_id>()) {
               world::sector* sector =
                  world::find_entity(_world.sectors, selected.get<world::sector_id>());

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
            else if (selected.is<world::portal_id>()) {
               world::portal* portal =
                  world::find_entity(_world.portals, selected.get<world::portal_id>());

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
            else if (selected.is<world::hintnode_id>()) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     selected.get<world::hintnode_id>());

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
            else if (selected.is<world::barrier_id>()) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     selected.get<world::barrier_id>());

               if (barrier) {
                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&barrier->position, new_position));
                  }
               }
            }
            else if (selected.is<world::planning_hub_id>()) {
               world::planning_hub* planning_hub =
                  world::find_entity(_world.planning_hubs,
                                     selected.get<world::planning_hub_id>());

               if (planning_hub) {
                  if (match_position) {
                     bundled_edits.push_back(
                        edits::make_set_value(&planning_hub->position, new_position));
                  }
               }
            }
            else if (selected.is<world::boundary_id>()) {
               world::boundary* boundary =
                  world::find_entity(_world.boundaries,
                                     selected.get<world::boundary_id>());

               if (boundary) {
                  bundled_edits.push_back(
                     edits::make_set_value(&boundary->position, new_position));
               }
            }
            else if (selected.is<world::measurement_id>()) {
               world::measurement* measurement =
                  world::find_entity(_world.measurements,
                                     selected.get<world::measurement_id>());

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
                        edits::make_set_block_box_metrics(*block_index, new_rotation,
                                                          new_position, box.size));
                  } break;
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