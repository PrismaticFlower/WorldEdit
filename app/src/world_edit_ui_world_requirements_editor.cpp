#include "world_edit.hpp"

#include "edits/add_world_req_entry.hpp"
#include "edits/add_world_req_list.hpp"
#include "edits/delete_world_req_entry.hpp"
#include "edits/delete_world_req_list.hpp"
#include "edits/set_world_req_entry.hpp"
#include "imgui_ext.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

enum class classification {
   generic,
   world_ref,
   layer_ref,
   gamemode_ref,
};

auto classify(const world::requirement_list& list) noexcept -> classification
{
   if (string::iequals(list.file_type, "path")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "congraph")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "envfx")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "world")) {
      return classification::layer_ref;
   }
   if (string::iequals(list.file_type, "prop")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "povs")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "lvl")) {
      return classification::gamemode_ref;
   }

   return classification::generic;
}

bool is_editable(const classification list_classification,
                 std::string_view entry, const world::world& world) noexcept
{
   if (list_classification == classification::world_ref) {
      return not string::iequals(entry, world.name);
   }
   else if (list_classification == classification::layer_ref) {
      auto [left, right] = string::split_first_of_exclusive(entry, "_");

      if (string::iequals(entry, world.name)) return false;
      if (not string::iequals(left, world.name)) return true;

      for (const world::layer_description& layer : world.layer_descriptions) {
         if (string::iequals(right, layer.name)) return false;
      }
   }
   else if (list_classification == classification::gamemode_ref) {
      auto [left, right] = string::split_first_of_exclusive(entry, "_");

      if (string::iequals(entry, world.name)) return false;
      if (not string::iequals(left, world.name)) return true;

      for (const world::game_mode_description& game_mode : world.game_modes) {
         if (string::iequals(right, game_mode.name)) return false;
      }
   }

   return true;
}

auto classification_string(const classification list_classification) -> const char*
{
   switch (list_classification) {
   case classification::world_ref:
      return "[World Dependency]";
   case classification::layer_ref:
      return "[Layer Dependency]";
   case classification::gamemode_ref:
      return "[Game Mode Dependency]";
   }

   return "";
}

}

void world_edit::ui_show_world_requirements_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({400.0f * _display_scale, 256.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({400.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("World Requirements (.req) Editor",
                    &_world_requirements_editor_open)) {
      ImGui::SeparatorText(".req Sections");

      for (int list_index = 0; list_index < _world.requirements.size(); ++list_index) {
         ImGui::PushID(list_index);

         const world::requirement_list& list = _world.requirements[list_index];

         if (ImGui::TreeNode(list.file_type.c_str())) {

            if (list.platform != world::platform::all) {
               switch (list.platform) {
               case world::platform::pc: {
                  ImGui::LabelText("Platform", "PC");
               } break;
               case world::platform::xbox: {
                  ImGui::LabelText("Platform", "Xbox");
               } break;
               case world::platform::ps2: {
                  ImGui::LabelText("Platform", "PS2");
               } break;
               }
            }

            if (list.alignment != 0) {
               ImGui::Value("Alignment", list.alignment);
            }

            ImGui::SeparatorText("Required Files");

            const classification list_classification = classify(list);

            if (ImGui::BeginTable("Required Files", 2)) {

               for (int entry_index = 0; entry_index < list.entries.size();
                    ++entry_index) {
                  ImGui::PushID(entry_index);

                  ImGui::TableNextRow();

                  if (is_editable(list_classification,
                                  list.entries[entry_index], _world)) {
                     ImGui::TableNextColumn();

                     if (absl::InlinedVector<char, 256>
                            entry{list.entries[entry_index].begin(),
                                  list.entries[entry_index].end()};
                         ImGui::InputText("##file", &entry)) {
                        if (not entry.empty()) {
                           _edit_stack_world.apply(edits::make_set_world_req_entry(
                                                      list_index, entry_index,
                                                      {entry.data(), entry.size()}),
                                                   _edit_context);
                        }
                     }

                     if (ImGui::IsItemDeactivatedAfterEdit()) {
                        _edit_stack_world.close_last();
                     }

                     ImGui::TableNextColumn();

                     if (ImGui::Button("Delete")) {
                        _edit_stack_world.apply(edits::make_delete_world_req_entry(
                                                   list_index, entry_index, _world),
                                                _edit_context);
                     }
                  }
                  else {
                     ImGui::TableNextColumn();

                     ImGui::TextUnformatted(list.entries[entry_index].c_str());

                     ImGui::TableNextColumn();

                     ImGui::TextUnformatted(classification_string(list_classification));
                  }

                  ImGui::PopID();
               }

               ImGui::EndTable();
            }

            ImGui::SeparatorText("Add File");

            ImGui::InputTextWithHint("##add", "i.e test_map", &_req_editor_add_entry);
            ImGui::SameLine();

            const bool can_add_file = not _req_editor_add_entry.empty();

            if (not can_add_file) ImGui::BeginDisabled();

            if (ImGui::Button("Add")) {
               _edit_stack_world.apply(edits::make_add_world_req_entry(list_index,
                                                                       std::move(_req_editor_add_entry)),
                                       _edit_context);

               _req_editor_add_entry = "";
            }

            if (not can_add_file) ImGui::EndDisabled();

            ImGui::SeparatorText("Delete Section");

            if (ImGui::Button("Delete")) {
               _edit_stack_world.apply(edits::make_delete_world_req_list(list_index, _world),
                                       _edit_context);
            }

            ImGui::TreePop();
         }

         ImGui::PopID();
      }

      ImGui::SeparatorText("Add New File Type");

      ImGui::InputTextWithHint("##create", "i.e model", &_req_editor_new_name);
      ImGui::SameLine();

      const bool can_add_type = not _req_editor_new_name.empty();

      if (not can_add_type) ImGui::BeginDisabled();

      if (ImGui::Button("Add")) {
         _edit_stack_world.apply(edits::make_add_world_req_list(
                                    std::move(_req_editor_new_name)),
                                 _edit_context);

         _req_editor_new_name = "";
      }

      if (not can_add_type) ImGui::EndDisabled();
   }

   ImGui::End();
}

}