#include "world_edit.hpp"

#include "edits/add_branch_weight.hpp"
#include "world/utility/world_utilities.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_world_selection_add_branch_weight() noexcept
{
   world::planning_hub* hub =
      world::find_entity(_world.planning_hubs,
                         _selection_add_branch_weight_context.from_hub_id);

   if (not hub) {
      _selection_edit_tool = selection_edit_tool::none;
      return;
   }

   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   bool open = _selection_edit_tool == selection_edit_tool::add_branch_weight;

   if (ImGui::Begin("Add Branch Weight", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      const world::planning_connection* connection =
         _selection_add_branch_weight_context.step == add_branch_weight_step::target
            ? world::find_entity(_world.planning_connections,
                                 _selection_add_branch_weight_context.connection_id)
            : nullptr;

      const world::planning_connection* candidate_connection = nullptr;
      const world::planning_hub* target_hub = nullptr;

      switch (_selection_add_branch_weight_context.step) {
      case add_branch_weight_step::connection: {
         ImGui::TextWrapped("Pick a connection for the branch weight.");

         if (_interaction_targets.hovered_entity and
             _interaction_targets.hovered_entity->is<world::planning_connection_id>()) {
            candidate_connection =
               world::find_entity(_world.planning_connections,
                                  _interaction_targets.hovered_entity
                                     ->get<world::planning_connection_id>());

            if (candidate_connection and
                _world.planning_hubs[candidate_connection->start_hub_index].id !=
                   hub->id and
                _world.planning_hubs[candidate_connection->end_hub_index].id !=
                   hub->id) {
               candidate_connection = nullptr;
               _interaction_targets.hovered_entity = {};
            }
         }
      } break;
      case add_branch_weight_step::target: {
         const char* hub_name = "<missing hub>";

         if (connection) {
            if (_world.planning_hubs[connection->start_hub_index].id ==
                _selection_add_branch_weight_context.from_hub_id) {
               hub_name =
                  _world.planning_hubs[connection->start_hub_index].name.c_str();
            }
            else {
               hub_name =
                  _world.planning_hubs[connection->end_hub_index].name.c_str();
            }
         }

         ImGui::TextWrapped(
            "Pick a destination hub for the branch weight. Can not be %s.", hub_name);

         if (_interaction_targets.hovered_entity and
             _interaction_targets.hovered_entity->is<world::planning_hub_id>()) {
            target_hub = world::find_entity(_world.planning_hubs,
                                            _interaction_targets.hovered_entity
                                               ->get<world::planning_hub_id>());

            if (connection and target_hub and
                (_world.planning_hubs[connection->start_hub_index].id ==
                    target_hub->id or
                 _world.planning_hubs[connection->end_hub_index].id ==
                    target_hub->id)) {
               target_hub = nullptr;
               _interaction_targets.hovered_entity = {};
            }
         }
      } break;
      }

      if (std::exchange(_selection_add_branch_weight_context.clicked, false)) {
         switch (_selection_add_branch_weight_context.step) {
         case add_branch_weight_step::connection: {
            if (candidate_connection) {
               _selection_add_branch_weight_context.step =
                  add_branch_weight_step::target;
               _selection_add_branch_weight_context.connection_id =
                  candidate_connection->id;
            }
         } break;
         case add_branch_weight_step::target: {
            if (not connection) {
               _selection_add_branch_weight_context.step =
                  add_branch_weight_step::connection;

               break;
            }

            if (target_hub) {
               const uint32 target_hub_index =
                  static_cast<uint32>(target_hub - _world.planning_hubs.data());
               const uint32 connection_index = static_cast<uint32>(
                  connection - _world.planning_connections.data());

               bool exists = false;

               for (const world::planning_branch_weights& weights : hub->weights) {
                  if (weights.hub_index == target_hub_index and
                      weights.connection_index == connection_index) {
                     exists = true;

                     break;
                  }
               }

               if (not exists) {
                  _edit_stack_world.apply(edits::make_add_branch_weight(
                                             &hub->weights,
                                             world::planning_branch_weights{
                                                .hub_index = target_hub_index,
                                                .connection_index = connection_index}),
                                          _edit_context);
               }

               _selection_add_branch_weight_context = {};

               open = false;
            }
         } break;
         }
      }

      if (connection) {
         _tool_visualizers.add_highlight(connection->id, _settings.graphics.hover_color);
      }
   }

   if (not open) _selection_edit_tool = selection_edit_tool::none;

   ImGui::End();
}
}