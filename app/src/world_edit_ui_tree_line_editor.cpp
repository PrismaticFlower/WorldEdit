#include "utility/string_icompare.hpp"
#include "world/utility/world_utilities.hpp"
#include "world_edit.hpp"

#include "edits/add_tree_line.hpp"
#include "edits/add_tree_line_border_odf.hpp"
#include "edits/delete_tree_line.hpp"
#include "edits/delete_tree_line_border_odf.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/set_tree_line_border_odf.hpp"

#include "imgui_ext.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_tree_line_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({640.0f * _display_scale, 698.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({640.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Tree Lines", &_tree_line_editor_open)) {
      if (ImGui::BeginChild("Tree Lines", {160.0f * _display_scale, 0.0f},
                            ImGuiChildFlags_ResizeX)) {
         ImGui::SeparatorText("Tree Lines");

         if (ImGui::BeginChild("##scroll_region",
                               {0.0f, ImGui::GetContentRegionAvail().y -
                                         ImGui::GetFrameHeightWithSpacing() * 2.0f})) {
            for (int32 i = 0; i < std::ssize(_world.tree_lines); ++i) {
               const world::tree_line& tree_line = _world.tree_lines[i];

               ImGui::PushID(i);

               if (ImGui::Selectable(_world.paths[tree_line.path_index].name.c_str(),
                                     _tree_line_editor_context.selected.id ==
                                        tree_line.id)) {
                  _tree_line_editor_context.selected = {.id = tree_line.id};
               }

               if (ImGui::IsItemHovered()) {
                  _interaction_targets.hovered_entity =
                     make_path_id_node_mask(_world.paths[tree_line.path_index].id);

                  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_MouseLeft)) {
                     _interaction_targets.selection.add(make_path_id_node_mask(
                        _world.paths[tree_line.path_index].id));

                     ImGui::SetWindowFocus("Selection");
                  }
               }

               ImGui::PopID();
            }
         }

         ImGui::EndChild();

         ImGui::BeginDisabled(_tree_line_editor_context.selected.id ==
                              world::tree_line_id{world::max_id});

         if (ImGui::Button("Delete", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            world::tree_line* selected_tree_line =
               world::find_entity(_world.tree_lines,
                                  _tree_line_editor_context.selected.id);

            if (selected_tree_line) {
               _edit_stack_world
                  .apply(edits::make_delete_tree_line(static_cast<uint32>(
                                                         selected_tree_line -
                                                         _world.tree_lines.data()),
                                                      _object_classes),
                         _edit_context);
            }

            _tree_line_editor_context.selected = {};
         }

         ImGui::EndDisabled();

         ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

         ImGui::BeginDisabled(_world.tree_lines.size() == world::max_tree_lines);

         if (ImGui::BeginCombo("##new_tree_line", "New Tree Line",
                               ImGuiComboFlags_HeightLargest)) {
            for (const world::path& path : _world.paths) {
               if (ImGui::Selectable(path.name.c_str())) {
                  _edit_stack_world.apply(
                     edits::make_add_tree_line(
                        {.border_odfs = {{"", world::object_class_library::null_handle()}},
                         .id = _world.next_id.tree_lines.aquire()},
                        _object_classes),
                     _edit_context);
               }

               if (ImGui::IsItemHovered()) {
                  _interaction_targets.hovered_entity =
                     make_path_id_node_mask(path.id);

                  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_MouseLeft)) {
                     _interaction_targets.selection.add(
                        make_path_id_node_mask(path.id));

                     ImGui::SetWindowFocus("Selection");
                  }
               }
            }
            ImGui::EndCombo();
         }

         ImGui::EndDisabled();
      }

      ImGui::EndChild();

      ImGui::SameLine();

      world::tree_line* selected_tree_line =
         world::find_entity(_world.tree_lines,
                            _tree_line_editor_context.selected.id);

      if (ImGui::BeginChild("##selected") and selected_tree_line) {
         ImGui::SeparatorText("Tree Line");

         ImGui::LabelText("Path", "%s",
                          _world.paths[selected_tree_line->path_index].name.c_str());

         ImGui::DragFloat("Spacing", &selected_tree_line->distance,
                          _edit_stack_world, _edit_context, 1.0f, 1.0f, 1e10f,
                          "%.3f", ImGuiSliderFlags_AlwaysClamp);

         ImGui::Checkbox("Flip", &selected_tree_line->flip, _edit_stack_world,
                         _edit_context);

         for (uint32 i = 0; i < selected_tree_line->border_odfs.size(); ++i) {
            ImGui::PushID((int)i);

            if (i == 0) {
               if (std::optional<lowercase_string> picked_object =
                      ui_object_class_pick_widget_untracked(
                         lowercase_string{selected_tree_line->border_odfs[i].name},
                         nullptr, "Border ODF");
                   picked_object) {
                  _edit_stack_world.apply(edits::make_set_tree_line_border_odf(
                                             &selected_tree_line->border_odfs, i,
                                             {*picked_object}, _object_classes),
                                          _edit_context);
               }
            }
            else {
               ImGui::SetNextItemWidth(ImGui::CalcItemWidth() -
                                       ImGui::CalcTextSize("X").x -
                                       ImGui::GetStyle().FramePadding.x * 2.0f -
                                       ImGui::GetStyle().ItemInnerSpacing.x);

               if (std::optional<lowercase_string> picked_object =
                      ui_object_class_pick_widget_untracked(
                         lowercase_string{selected_tree_line->border_odfs[i].name},
                         nullptr, "##border_odf");
                   picked_object) {
                  _edit_stack_world.apply(edits::make_set_tree_line_border_odf(
                                             &selected_tree_line->border_odfs, i,
                                             {*picked_object}, _object_classes),
                                          _edit_context);
               }

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

               if (ImGui::Button("X")) {
                  _edit_stack_world.apply(edits::make_delete_tree_line_border_odf(
                                             &selected_tree_line->border_odfs,
                                             i, _object_classes),
                                          _edit_context);
               }

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

               ImGui::Text("Border ODF");
            }

            ImGui::PopID();
         }

         ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_Button));
         ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,
                               ImGui::GetColorU32(ImGuiCol_ButtonHovered));
         ImGui::PushStyleColor(ImGuiCol_FrameBgActive,
                               ImGui::GetColorU32(ImGuiCol_ButtonActive));

         ImGui::BeginDisabled(selected_tree_line->border_odfs.size() ==
                              world::max_tree_line_border_odfs);

         if (std::optional<lowercase_string> picked_class =
                ui_object_class_pick_widget_untracked({}, "", "##odf_picker");
             picked_class) {
            _edit_stack_world.apply(edits::make_add_tree_line_border_odf(
                                       &selected_tree_line->border_odfs,
                                       {*picked_class}, _object_classes),
                                    _edit_context);
         }

         ImGui::PopStyleColor(3);

         const ImVec2 add_min = ImGui::GetItemRectMin();
         const ImVec2 add_text_size = ImGui::CalcTextSize("Add Border ODF");

         const ImVec2 add_text_offset =
            {roundf((ImGui::CalcItemWidth() - add_text_size.x) *
                    ImGui::GetStyle().ButtonTextAlign.x),
             roundf((ImGui::GetFrameHeight() - add_text_size.y) *
                    ImGui::GetStyle().ButtonTextAlign.y)};

         ImGui::GetWindowDrawList()->AddText({add_min.x + add_text_offset.x,
                                              add_min.y + add_text_offset.y},
                                             ImGui::GetColorU32(ImGuiCol_Text),
                                             "Add Border ODF");

         ImGui::SetItemTooltip(
            "Warning: Multiple border ODFs will not preview accurately in "
            "WorldEdit. And the order of chosen ODFs is random ingame and can "
            "change from unrelated edits you make.");

         ImGui::EndDisabled();

         if (selected_tree_line->border_odfs.size() > 1) {
            ImGui::TextWrapped(
               "Warning: Multiple border ODFs will not preview accurately in "
               "WorldEdit. And the order of chosen ODFs is random ingame and "
               "can change from unrelated edits you make.");
         }
      }

      ImGui::EndChild();
   }

   ImGui::End();
}

}