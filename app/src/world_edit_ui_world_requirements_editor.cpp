#include "world_edit.hpp"

#include "edits/add_world_req_entry.hpp"
#include "edits/add_world_req_list.hpp"
#include "edits/delete_world_req_entry.hpp"
#include "edits/delete_world_req_list.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

void world_edit::ui_show_world_requirements_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({400.0f * _display_scale, 256.0f * _display_scale},
                                       {512.0f * _display_scale,
                                        1024.0f * _display_scale});

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

            for (int i = 0; i < list.entries.size(); ++i) {
               ImGui::LabelText("##file", list.entries[i].c_str());

               ImGui::SameLine();

               ImGui::PushID(i);

               if (ImGui::Button("Delete")) {
                  _edit_stack_world.apply(edits::make_delete_world_req_entry(list_index,
                                                                             i, _world),
                                          _edit_context);
               }

               ImGui::PopID();
            }

            ImGui::SeparatorText("Add File");

            ImGui::InputTextWithHint("##add", "i.e test_map", &_req_editor_add_entry,
                                     ImGuiInputTextFlags_CharsNoBlank);
            ImGui::SameLine();

            if (ImGui::Button("Add")) {
               _edit_stack_world.apply(edits::make_add_world_req_entry(list_index,
                                                                       std::move(_req_editor_add_entry)),
                                       _edit_context);

               _req_editor_add_entry = "";
            }

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

      ImGui::InputTextWithHint("##create", "i.e model", &_req_editor_new_name,
                               ImGuiInputTextFlags_CharsNoBlank);
      ImGui::SameLine();

      if (ImGui::Button("Add")) {
         _edit_stack_world.apply(edits::make_add_world_req_list(
                                    std::move(_req_editor_new_name)),
                                 _edit_context);

         _req_editor_new_name = "";
      }
   }

   ImGui::End();
}

}