#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"

#include "world/blocks/accessors.hpp"
#include "world/blocks/find.hpp"

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
         for (int8 i = 0; i < _world.layer_descriptions.size(); ++i) {
            if (ImGui::Selectable(_world.layer_descriptions[i].name.c_str())) {
               _selection_set_layer = i;
            }
         }

         ImGui::EndCombo();
      }

      if (ImGui::Button("Set", {ImGui::CalcItemWidth(), 0.0f})) {
         edits::bundle_vector bundled_edits;

         for (const auto& selected : _interaction_targets.selection) {
            if (selected.is<world::object_id>()) {
               world::object* object =
                  world::find_entity(_world.objects, selected.get<world::object_id>());

               if (object and object->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&object->layer, _selection_set_layer));
               }
            }
            else if (selected.is<world::path_id_node_mask>()) {
               const auto& [id, node_mask] =
                  selected.get<world::path_id_node_mask>();

               world::path* path = world::find_entity(_world.paths, id);

               if (path and path->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&path->layer, _selection_set_layer));
               }
            }
            else if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (light and light->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&light->layer, _selection_set_layer));
               }
            }
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

               if (region and region->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&region->layer, _selection_set_layer));
               }
            }
            else if (selected.is<world::hintnode_id>()) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     selected.get<world::hintnode_id>());

               if (hintnode and hintnode->layer != _selection_set_layer) {
                  bundled_edits.push_back(
                     edits::make_set_value(&hintnode->layer, _selection_set_layer));
               }
            }
            else if (selected.is<world::block_id>()) {
               const world::block_id block_id = selected.get<world::block_id>();
               const std::optional<uint32> block_index =
                  world::find_block(_world.blocks, block_id);

               if (block_index) {
                  int8& block_layer =
                     world::get_block_layer(_world.blocks, block_id.type(), *block_index);

                  if (block_layer != _selection_set_layer) {
                     bundled_edits.push_back(
                        edits::make_set_value(&block_layer, _selection_set_layer));
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