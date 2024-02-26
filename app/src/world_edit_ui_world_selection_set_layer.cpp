#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_world_selection_set_layer() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = _selection_edit_tool == selection_edit_tool::set_layer;

   if (ImGui::Begin("Set Layer##Selection", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      if (_selection_set_layer > _world.layer_descriptions.size()) {
         _selection_set_layer = 0;
      }

      if (ImGui::BeginCombo("Layer",
                            _world.layer_descriptions[_selection_set_layer].name.c_str())) {
         for (int16 i = 0; i < _world.layer_descriptions.size(); ++i) {
            if (ImGui::Selectable(_world.layer_descriptions[i].name.c_str())) {
               _selection_set_layer = i;
            }
         }

         ImGui::EndCombo();
      }

      if (ImGui::Button("Set", {ImGui::CalcItemWidth(), 0.0f})) {
         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (std::holds_alternative<world::object_id>(selected)) {
               world::object* object =
                  world::find_entity(_world.objects,
                                     std::get<world::object_id>(selected));

               if (object and object->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&object->layer, _selection_set_layer));
               }
            }
            else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
               const auto [id, node_index] =
                  std::get<world::path_id_node_pair>(selected);

               world::path* path = world::find_entity(_world.paths, id);

               if (path and path->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&path->layer, _selection_set_layer));
               }
            }
            else if (std::holds_alternative<world::light_id>(selected)) {
               world::light* light =
                  world::find_entity(_world.lights,
                                     std::get<world::light_id>(selected));

               if (light and light->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&light->layer, _selection_set_layer));
               }
            }
            else if (std::holds_alternative<world::region_id>(selected)) {
               world::region* region =
                  world::find_entity(_world.regions,
                                     std::get<world::region_id>(selected));

               if (region and region->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&region->layer, _selection_set_layer));
               }
            }
            else if (std::holds_alternative<world::hintnode_id>(selected)) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     std::get<world::hintnode_id>(selected));

               if (hintnode and hintnode->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&hintnode->layer, _selection_set_layer));
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